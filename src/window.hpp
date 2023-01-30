#pragma once


namespace cwt {

class window {
public:
    window() {
        init();
    }
    ~window() {
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }
    GLFWwindow* get_window() { return m_window; }
private:
    void init(){
        glfwInit();
        // don't use opengl!
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        m_window = glfwCreateWindow(m_width, m_height, m_name.c_str(), nullptr, nullptr);
    }
private:
    GLFWwindow* m_window;
    std::size_t m_width{800};
    std::size_t m_height{600};
    std::string m_name{"Test Window"};
};

} // namespace cwt