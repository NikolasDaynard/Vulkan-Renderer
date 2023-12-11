#ifndef GAMEFUNCTIONS_H_INCLUDED
#define GAMEFUNCTIONS_H_INCLUDED
int getFPS(int &it, std::chrono::time_point<std::chrono::high_resolution_clock> start, std::chrono::time_point<std::chrono::high_resolution_clock> end) {
    auto elapsedTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    ++it;

    if (elapsedTime >= 1000000) {
        double fps = static_cast<double>(it) / (elapsedTime / 1000000);
        std::cout << "FPS: " << fps << "\n";
        it = 0;
        return 1;
    }
    return 0;
}
#endif
