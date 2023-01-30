#pragma once

namespace cwt 
{

class vulkan_renderer 
{
public:
    vulkan_renderer() = delete;
    vulkan_renderer(GLFWwindow* window) : m_window(window) {
        if (init() == EXIT_FAILURE) {
            throw std::runtime_error("vulkan renderer failed to initialize");
        }
    }
    ~vulkan_renderer(){
        cleanup();
    }
private:
    int init()
    {
        try {
            // order matters!
            create_instance();
            get_physical_device();
            create_logical_device();
        } catch(const std::runtime_error& e) {
            std::cerr << e.what() << '\n';
            return EXIT_FAILURE;
        }
        return 0;
    }
    void cleanup() 
    {
        vkDestroyDevice(m_main_device.logical_device, nullptr);
        vkDestroyInstance(m_instance, nullptr);
    }
    void create_instance()
    {
        if (this->m_enable_validation_layers && !check_validation_support()) {
            throw std::runtime_error("Validation layers requested, but not available!");
        }

        // some infos, most likely dont affect the program -> more for developers purpose
        VkApplicationInfo app_info{};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = "Vulkan App";
        app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.pEngineName = "no engine";
        app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.apiVersion = VK_API_VERSION_1_3;

        VkInstanceCreateInfo create_info{}; 
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pApplicationInfo = &app_info;
        
        uint32_t glfw_extension_count = 0;
        const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
        
        std::vector<const char*> instance_extensions; 
        for (std::size_t i = 0 ; i < glfw_extension_count ; i++){
            instance_extensions.push_back(glfw_extensions[i]);
        }

        if(!check_instance_extension_support(instance_extensions)) {
            throw std::runtime_error("VkInstance does not support required extension");
        }

        create_info.enabledExtensionCount = static_cast<uint32_t>(instance_extensions.size());
        create_info.ppEnabledExtensionNames = instance_extensions.data();

        if (this->m_enable_validation_layers) {
            create_info.enabledLayerCount = static_cast<uint32_t>(m_validation_layers.size());
            create_info.ppEnabledLayerNames = m_validation_layers.data();
        } else {
            create_info.enabledLayerCount = 0;
            create_info.ppEnabledLayerNames = nullptr;
        }

        auto result = vkCreateInstance(&create_info, nullptr, &m_instance);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create vulkan instance");
        }
    }
    void get_physical_device()
    {
        uint32_t device_count = 0;
        vkEnumeratePhysicalDevices(m_instance, &device_count, nullptr);

        if (device_count == 0) {
            throw std::runtime_error("Can't find GPUs that support vulkan instance");
        }

        std::vector<VkPhysicalDevice> device_list(device_count);
        vkEnumeratePhysicalDevices(m_instance, &device_count, device_list.data());

        for (const auto& d : device_list){
            if(check_device_suitable(d)){
                m_main_device.physical_device = d;
                break;
            }
        }
    }
    void create_logical_device()
    {
        auto indices = get_queue_families(m_main_device.physical_device);
        VkDeviceQueueCreateInfo queue_info{};
        queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_info.queueFamilyIndex = indices.graphics_family;
        queue_info.queueCount = 1;
        float priority = 1.0f;
        queue_info.pQueuePriorities = &priority;

        VkDeviceCreateInfo device_info{};
        device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_info.queueCreateInfoCount = 1;
        device_info.pQueueCreateInfos = &queue_info;
        device_info.enabledExtensionCount = 0;
        device_info.ppEnabledExtensionNames = nullptr;

        VkPhysicalDeviceFeatures device_features{};
        device_info.pEnabledFeatures = &device_features;

        VkResult result = vkCreateDevice(m_main_device.physical_device, &device_info, nullptr, &m_main_device.logical_device);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create a logical device");
        }
        vkGetDeviceQueue(m_main_device.logical_device, indices.graphics_family, 0, &m_graphics_queue);
    }
    bool check_instance_extension_support(const std::vector<const char*>& extensions_to_check)
    {
        uint32_t extension_count = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

        std::vector<VkExtensionProperties> extensions(extension_count);
        vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());

        for(const auto& e1 : extensions_to_check){
            bool has_extension = false;
            for (const auto& e2 : extensions) {
                if (strcmp(e1, e2.extensionName)){
                    has_extension = true;
                    break;
                }
            }
            if (!has_extension) {
                return false;
            }
        }
        return true;
    }
    bool check_device_suitable(VkPhysicalDevice device) 
    {
        // // check device infos, name, type, vendor, etc.
        // VkPhysicalDeviceProperties device_properties;
        // vkGetPhysicalDeviceProperties(device, &device_properties);

        // // check device features, geo shader, tess shader, wide lines, etc
        // VkPhysicalDeviceFeatures device_features;   
        // vkGetPhysicalDeviceFeatures(device, &device_features);
        
        return get_queue_families(device).is_valid();
    }
    queue_family_indices get_queue_families(VkPhysicalDevice device)
    {
        uint32_t count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);

        std::vector<VkQueueFamilyProperties> queue_family_list(count);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &count, queue_family_list.data());

        queue_family_indices indices;
        for (std::size_t idx = 0 ; const auto& q : queue_family_list){
            if (q.queueCount > 0 && q.queueFlags & VK_QUEUE_GRAPHICS_BIT){
                indices.graphics_family = idx;
            }
            if (indices.is_valid()){
                break;
            }
            idx++;
        }
        return indices;
    }
    bool check_validation_support()
    {
        uint32_t count;
        vkEnumerateInstanceLayerProperties(&count, nullptr);
    
        std::vector<VkLayerProperties> available_layers(count);
        vkEnumerateInstanceLayerProperties(&count, available_layers.data());
            
        for (const char* layer : m_validation_layers) {
            bool found = false;
            for (const auto& properties : available_layers) {
                if (strcmp(layer, properties.layerName) == 0) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                return false;
            }
        }
    
        return true;
    }

private:
    GLFWwindow* m_window;
    VkInstance m_instance;
    struct {
        VkPhysicalDevice physical_device;
        VkDevice logical_device;
    } m_main_device;
    VkQueue m_graphics_queue;

#ifdef NDEBUG
    constexpr static const bool m_enable_validation_layers = false;
#else
    constexpr static const bool m_enable_validation_layers = true;
#endif 
    const std::vector<const char*> m_validation_layers = {
        "VK_LAYER_KHRONOS_validation"
    };
};

} // namespace cwt

