#pragma once
#include <filesystem>
void openInDefaultApp(const std::filesystem::path &path);

// Lance un executable (dossier de travail = dossier de l'exe).
void launchProgram(const std::filesystem::path &exePath);