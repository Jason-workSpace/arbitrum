/*
 * Copyright 2021, Offchain Labs, Inc.
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
 */

#include <avm_values/unloadedvalue.hpp>

bool UnloadedValue::isHeaped() const {
    return impl.heaped_value.zero == 0;
}

const HeapedUnloadedValueInfo& UnloadedValue::getHeaped() const {
    assert(isHeaped() == 0);
    return *impl.heaped_value.ptr;
}

UnloadedValue::UnloadedValue(BigUnloadedValue big)
    : impl{InlineUnloadedValue{}} {
    assert(big.value_size > 0);
    // Attempt to inline it
    if (big.type == ValueTypes::TUPLE) {
        auto small_size = uint64_t(big.value_size);
        if (uint256_t(small_size) == big.value_size && small_size > 0) {
            impl.inline_value.value_size = small_size;
            impl.inline_value.hash = big.hash;
            return;
        }
    }

    // We can't inline this; put it in a shared_ptr
    impl.heaped_value.zero = 0;
    impl.heaped_value.type = big.type;
    impl.heaped_value.ptr = std::make_shared<HeapedUnloadedValueInfo>(
        HeapedUnloadedValueInfo{big.hash, big.value_size});
}

UnloadedValue::~UnloadedValue() {
    if (isHeaped()) [[unlikely]] {
        impl.heaped_value.ptr.~shared_ptr();
    }
}

UnloadedValue::UnloadedValue(const UnloadedValue& other)
    : impl{InlineUnloadedValue{}} {
    if (other.isHeaped()) [[unlikely]] {
        impl.heaped_value.zero = 0;
        impl.heaped_value.type = other.impl.heaped_value.type;
        impl.heaped_value.ptr = other.impl.heaped_value.ptr;
    } else {
        impl.inline_value.value_size = other.impl.inline_value.value_size;
        impl.inline_value.hash = other.impl.inline_value.hash;
    }
}

UnloadedValue& UnloadedValue::operator=(const UnloadedValue& other) {
    *this = other;
    return *this;
}

UnloadedValue::UnloadedValue(UnloadedValue&& other)
    : impl{InlineUnloadedValue{}} {
    if (other.isHeaped()) [[unlikely]] {
        impl.heaped_value.zero = 0;
        impl.heaped_value.type = other.impl.heaped_value.type;
        impl.heaped_value.ptr = std::move(other.impl.heaped_value.ptr);
    } else {
        impl.inline_value.value_size = other.impl.inline_value.value_size;
        impl.inline_value.hash = other.impl.inline_value.hash;
    }
}

UnloadedValue& UnloadedValue::operator=(UnloadedValue&& other) {
    *this = std::move(other);
    return *this;
}

uint256_t UnloadedValue::hash() const {
    if (isHeaped()) [[unlikely]] {
        return getHeaped().hash;
    } else {
        return impl.inline_value.hash;
    }
}

uint256_t UnloadedValue::value_size() const {
    if (isHeaped()) [[unlikely]] {
        return getHeaped().value_size;
    } else {
        return impl.inline_value.value_size;
    }
}

ValueTypes UnloadedValue::type() const {
    if (isHeaped()) [[unlikely]] {
        return impl.heaped_value.type;
    } else {
        return ValueTypes::TUPLE;
    }
}