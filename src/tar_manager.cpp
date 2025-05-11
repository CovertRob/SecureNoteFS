/*
Responsibilities of tar_manager: 

Use either a simple system("tar czf …") approach or integrate libarchive for pure-C++ control.

bool create_timestamped(const std::string &dataDir, std::string &outFilename)

bool extract(const std::string &tarballPath, const std::string &dataDir)

Return false on error so main() can warn and preserve data/.
*/