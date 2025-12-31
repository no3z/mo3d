#include "core/Application.h"
#include "utils/Logger.h"
#include <exception>

int main(int argc, char** argv) {
    try {
        mo3d::Logger::Instance().SetLevel(mo3d::LogLevel::Info);

        mo3d::Application app;

        if (!app.Initialize()) {
            LOG_ERROR("Failed to initialize application");
            return 1;
        }

        app.Run();
        app.Shutdown();

        return 0;

    } catch (const std::exception& e) {
        LOG_ERROR("Fatal error: ", e.what());
        return 1;
    } catch (...) {
        LOG_ERROR("Unknown fatal error");
        return 1;
    }
}
