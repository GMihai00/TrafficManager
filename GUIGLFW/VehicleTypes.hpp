#ifndef GUI_VEHICLETYPES_HPP
#define GUI_VEHICLETYPES_HPP

#include <iostream>

namespace paint
{
    enum class VehicleTypes : uint8_t
    {
        VT_VEHICLE,
        NORMAL_VEHICLE,
        EMERGENCY_VEHICLE
    };
} // namespace paint
#endif // #GUI_VEHICLETYPES_HPP
