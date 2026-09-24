#ifdef _WIN32
#include "window.hpp"
int main() { return app::run(); }
#else
#include "server/http.hpp"
int main() { return server::run(); }
#endif
