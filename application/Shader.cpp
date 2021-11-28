#include "Shader.h"

#include <fstream>
#include <stdexcept>

Shader::Shader(std::string filePath)
    : m_FilePath(filePath)
{
    std::ifstream file(filePath, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    m_FileData.resize(fileSize);

    file.seekg(0);
    file.read(m_FileData.data(), fileSize);
    file.close();
}

vk::raii::ShaderModule Shader::GetShaderModule(const vk::raii::Device& device) const
{
    return vk::raii::ShaderModule(
        device,
        vk::ShaderModuleCreateInfo({}, m_FileData.size(), reinterpret_cast<const uint32_t*>(m_FileData.data())));
}
