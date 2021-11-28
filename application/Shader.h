#include <vulkan/vulkan_raii.hpp>

#include <string>
#include <vector>

class Shader {
    std::string       m_FilePath;
    std::vector<char> m_FileData;

  public:
    Shader(std::string filePath);
    ~Shader() = default;

    const std::vector<char>& GetFileData() const { return m_FileData; }
    vk::raii::ShaderModule   GetShaderModule(const vk::raii::Device& device) const;
};
