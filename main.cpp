#include "application/HelloTriangle.h"

#include <vulkan/vulkan.hpp>

#include <iostream>

int main(int /*argc*/, char** /*argv*/)
{
    try
    {
        HelloTriangle app;

        app.Run();
    }
    catch (vk::SystemError& err)
    {
        std::cout << "vk::SystemError: " << err.what() << std::endl;
        exit(-1);
    }
    catch (std::exception& err)
    {
        std::cout << "std::exception: " << err.what() << std::endl;
        exit(-1);
    }
    catch (...)
    {
        std::cout << "unknown error\n";
        exit(-1);
    }

    /* VULKAN_HPP_KEY_END */

    return 0;
}