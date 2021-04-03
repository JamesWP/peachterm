#include "io.hpp"

#include <chrono>
#include <ctype.h>
#include <iostream>
#include <string>

#include <Windows.h>
#include <process.h>

// Much of this code courtesy Microsoft/terminal developers
// https://github.com/microsoft/terminal/blob/main/samples/ConPTY/EchoCon/EchoCon/EchoCon.cpp

namespace {
HRESULT CreatePseudoConsoleAndPipes(HPCON *, HANDLE *, HANDLE *);
HRESULT InitializeStartupInfoAttachedToPseudoConsole(STARTUPINFOEX *, HPCON);
} // namespace

namespace io {
PseudoTerminal::PseudoTerminal(data_read_cb data_cb) : _data_cb(data_cb) {
  // TODO: implement
}

PseudoTerminal::~PseudoTerminal() {
  if (child_process_output != nullptr) {
    child_process_output = nullptr;
  }

  if (child_process != nullptr) {
    TerminateProcess(child_process, 255);
  }
  if (pseudo_terminal != nullptr) {
    ClosePseudoConsole(pseudo_terminal);
  }

  read_complete();
  t.join();
}

void PseudoTerminal::read_complete() {
  std::cout << "Read complete" << std::endl;
  {
    std::unique_lock<std::mutex> lk(read_data_state);
    buffer_ready_for_write = true;
  }
  read_data_state_cv.notify_one();
}

void PseudoTerminal::write(char data) { write(&data, 1u); }

void PseudoTerminal::write(std::string_view data) {
  write(data.data(), data.size());
}

void PseudoTerminal::write(const char *data, size_t len) {
  DWORD bytes_written;
  if (!WriteFile(child_process_input, data, (DWORD) len, &bytes_written, NULL)) {
    throw std::runtime_error("write error");
  }

  if (bytes_written != len) {
    std::cerr << "UNABLE TO WRITE FULL OUTPUT: " << bytes_written << " out of " << len << std::endl;
  }
}

bool PseudoTerminal::start() {

  
  HPCON hPC{INVALID_HANDLE_VALUE};

  //  Create the Pseudo Console and pipes to it
  HANDLE hPipeIn{INVALID_HANDLE_VALUE};
  HANDLE hPipeOut{INVALID_HANDLE_VALUE};
  
  HRESULT hr = CreatePseudoConsoleAndPipes(&hPC, &hPipeIn, &hPipeOut);

  if (S_OK != hr) {
    return false;
  }

  pseudo_terminal = hPC;
  child_process_output = hPipeIn;
  child_process_input = hPipeOut;

  return true;
}

bool PseudoTerminal::set_size(int rows, int cols) {
  COORD dim = {static_cast<short>(cols), static_cast<short>(rows)};

  auto hr = ResizePseudoConsole(pseudo_terminal, dim);
  if (S_OK != hr) {
    return false;
  }

  return true;
}

bool PseudoTerminal::fork_child() {
  // TODO: implement

  HRESULT hr{E_UNEXPECTED};
  std::string Command = "wsl.exe";

  // Initialize the necessary startup info struct
  STARTUPINFOEX startupInfo{};
  if (S_OK != InitializeStartupInfoAttachedToPseudoConsole(&startupInfo, pseudo_terminal)) {
    return false;
  }

  // Needed to stop our debugger (if there is one) from taking the output from
  // our child.
  startupInfo.StartupInfo.hStdError = NULL;
  startupInfo.StartupInfo.hStdInput = NULL;
  startupInfo.StartupInfo.hStdOutput = NULL;
  startupInfo.StartupInfo.dwFlags = STARTF_USESTDHANDLES;

  // Launch ping to emit some text back via the pipe
  PROCESS_INFORMATION piClient{};
  hr = CreateProcess(NULL,           // No module name - use Command Line
                     Command.data(), // Command Line
                     NULL,           // Process handle not inheritable
                     NULL,           // Thread handle not inheritable
                     FALSE,          // Inherit handles
                     EXTENDED_STARTUPINFO_PRESENT, // Creation flags
                     NULL, // Use parent's environment block
                     NULL, // Use parent's starting directory
                     &startupInfo.StartupInfo, // Pointer to STARTUPINFO
                     &piClient)                // Pointer to PROCESS_INFORMATION
           ? S_OK
           : GetLastError();

  if (S_OK != hr) {
    return false;
  }

  auto read_from_app = std::thread([this]() {
    const DWORD BUFF_SIZE{512};
    char szBuffer[BUFF_SIZE];

    DWORD dwBytesRead = 0;
    do {
      // Wait for buffer to be ready
      {
        std::cout << "Wating to read" << std::endl;
        std::unique_lock<std::mutex> lk(read_data_state);
        read_data_state_cv.wait(lk, [this] { return buffer_ready_for_write; });
        buffer_ready_for_write = false;
        
        std::cout << "Doing read" << std::endl;
        // it _was_ ready for write, but since we are about to send it, mark it
        // as not ready now...
      }

      if(child_process_output == nullptr) {
        break;
      }

      // Read from the pipe
      BOOL fRead = ReadFile(child_process_output, szBuffer, BUFF_SIZE, &dwBytesRead, NULL);

      
      std::cout << "Done read" << std::endl;

      if (!fRead) {
        break;
      }

      // Return buffer to sender, the sender will call `read_complete`
      _data_cb(this, szBuffer, dwBytesRead);
    } while (dwBytesRead >= 0);
  });

  std::swap(t, read_from_app);

  std::cout << "ProcessID: " << piClient.dwProcessId << std::endl;

  child_process = piClient.hProcess;

  return true;
}

} // namespace io
namespace {
HRESULT CreatePseudoConsoleAndPipes(HPCON *phPC, HANDLE *phPipeIn,
                                    HANDLE *phPipeOut) {
  HRESULT hr{E_UNEXPECTED};
  HANDLE hPipePTYIn{INVALID_HANDLE_VALUE};
  HANDLE hPipePTYOut{INVALID_HANDLE_VALUE};

  // Create the pipes to which the ConPTY will connect
  if (CreatePipe(&hPipePTYIn, phPipeOut, NULL, 0) &&
      CreatePipe(phPipeIn, &hPipePTYOut, NULL, 0)) {
    // Determine required size of Pseudo Console
    COORD consoleSize{80, 24};

    // Create the Pseudo Console of the required size, attached to the PTY-end
    // of the pipes
    hr = CreatePseudoConsole(consoleSize, hPipePTYIn, hPipePTYOut, 0, phPC);

    // Note: We can close the handles to the PTY-end of the pipes here
    // because the handles are dup'ed into the ConHost and will be released
    // when the ConPTY is destroyed.
    if (INVALID_HANDLE_VALUE != hPipePTYOut)
      CloseHandle(hPipePTYOut);
    if (INVALID_HANDLE_VALUE != hPipePTYIn)
      CloseHandle(hPipePTYIn);
  }

  return hr;
}

HRESULT
InitializeStartupInfoAttachedToPseudoConsole(STARTUPINFOEX *pStartupInfo,
                                             HPCON hPC) {
  HRESULT hr{E_UNEXPECTED};

  if (pStartupInfo) {
    size_t attrListSize{};

    pStartupInfo->StartupInfo.cb = sizeof(STARTUPINFOEX);

    // Get the size of the thread attribute list.
    InitializeProcThreadAttributeList(NULL, 1, 0, &attrListSize);

    // Allocate a thread attribute list of the correct size
    pStartupInfo->lpAttributeList =
        reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(malloc(attrListSize));

    // Initialize thread attribute list
    if (pStartupInfo->lpAttributeList &&
        InitializeProcThreadAttributeList(pStartupInfo->lpAttributeList, 1, 0,
                                          &attrListSize)) {
      // Set Pseudo Console attribute
      hr = UpdateProcThreadAttribute(pStartupInfo->lpAttributeList, 0,
                                     PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE, hPC,
                                     sizeof(HPCON), NULL, NULL)
               ? S_OK
               : HRESULT_FROM_WIN32(GetLastError());
    } else {
      hr = HRESULT_FROM_WIN32(GetLastError());
    }
  }
  return hr;
}
} // namespace