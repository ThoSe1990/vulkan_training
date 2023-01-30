#pragma once


namespace cwt {

    struct queue_family_indices {
        int graphics_family = -1; // location of graphics queue family
        bool is_valid() {
            return graphics_family >= 0;
        }
    };

} // namespace cwt