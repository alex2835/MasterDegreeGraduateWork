#define SDL_MAIN_HANDLED

#include "unfolding/unfolding_app.hpp"
#include "core/Debug/Instrumentor.hpp"
#include "core/Log.hpp"

int main() {
  try {
    APP_PROFILE_BEGIN_SESSION_WITH_FILE("Unfolding", "profile.json");

    {
      APP_PROFILE_SCOPE("Test scope");
      UnfoldingApp app{"Unfolding"};
      app.run();
    }

    APP_PROFILE_END_SESSION();
  } catch (std::exception& e) {
    APP_ERROR("Main process terminated with: {}", e.what());
  }

  return 0;
}
