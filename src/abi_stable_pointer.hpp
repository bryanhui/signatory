/* Copyright 2019 Patrick Kidger. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *    http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ========================================================================= */
 // ABI-stable smart pointer implementation to avoid C++ ABI compatibility issues
 // with std::unique_ptr between different compiler versions and C++ ABI settings.

#ifndef SIGNATORY_ABI_STABLE_POINTER_HPP
#define SIGNATORY_ABI_STABLE_POINTER_HPP

#include <cstddef>  // std::nullptr_t
#include <utility>  // std::forward

namespace signatory {
    namespace misc {
        // A simple, ABI-stable smart pointer with exclusive ownership semantics.
        // Similar to std::unique_ptr but uses only raw pointers internally,
        // making it immune to C++ ABI version mismatches.
        //
        // Key features:
        // - Move-only (no copy constructor/assignment)
        // - Automatic memory management via RAII
        // - Compatible with both old and new C++ ABIs
        // - Header-only implementation
        template<typename T>
        class AbiStablePtr {
        public:
            // Default constructor - null pointer
            constexpr AbiStablePtr() noexcept : ptr_(nullptr) {}
            
            // Construct from raw pointer (takes ownership)
            explicit AbiStablePtr(T* p) noexcept : ptr_(p) {}
            
            // Destructor - automatically deletes owned object
            ~AbiStablePtr() {
                delete ptr_;
            }
            
            // Disable copy operations (exclusive ownership)
            AbiStablePtr(const AbiStablePtr&) = delete;
            AbiStablePtr& operator=(const AbiStablePtr&) = delete;
            
            // Enable move operations
            AbiStablePtr(AbiStablePtr&& other) noexcept : ptr_(other.ptr_) {
                other.ptr_ = nullptr;  // Transfer ownership
            }
            
            AbiStablePtr& operator=(AbiStablePtr&& other) noexcept {
                if (this != &other) {
                    delete ptr_;         // Delete current object
                    ptr_ = other.ptr_;   // Take ownership
                    other.ptr_ = nullptr;
                }
                return *this;
            }
            
            // Reset - delete current object and take ownership of new one
            void reset(T* p = nullptr) noexcept {
                T* old = ptr_;
                ptr_ = p;
                delete old;
            }
            
            // Release - return raw pointer without deleting (transfers ownership to caller)
            T* release() noexcept {
                T* p = ptr_;
                ptr_ = nullptr;
                return p;
            }
            
            // Get raw pointer (doesn't transfer ownership)
            T* get() const noexcept {
                return ptr_;
            }
            
            // Dereference operators
            T& operator*() const {
                return *ptr_;
            }
            
            T* operator->() const {
                return ptr_;
            }
            
            // Boolean conversion
            explicit operator bool() const noexcept {
                return ptr_ != nullptr;
            }
            
            // Comparison with nullptr
            bool operator==(std::nullptr_t) const noexcept {
                return ptr_ == nullptr;
            }
            
            bool operator!=(std::nullptr_t) const noexcept {
                return ptr_ != nullptr;
            }
            
        private:
            T* ptr_;  // Raw pointer - ABI stable
        };
        
        // Helper function for creating AbiStablePtr (similar to std::make_unique)
        template<typename T, typename... Args>
        AbiStablePtr<T> make_abi_stable(Args&&... args) {
            return AbiStablePtr<T>(new T(std::forward<Args>(args)...));
        }
        
    }  // namespace signatory::misc
}  // namespace signatory

#endif  // SIGNATORY_ABI_STABLE_POINTER_HPP
