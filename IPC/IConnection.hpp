#pragma once

namespace ipc
{
    namespace net
    {
        template<typename T>
        class IConnection
        {
        public:
            virtual ~IConnection() noexcept = default;
        };
    }
}