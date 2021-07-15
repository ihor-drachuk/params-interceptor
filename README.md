# Params interceptor

Shows with what parameters the application is launched.

Instructions:
1. Rename target (studied) exe file: "file.exe" -> "file.exe_orig"
2. Download [Interceptor](https://github.com/ihor-drachuk/params-interceptor/releases/download/v0.2/interceptor_v0.2.exe) and put it inplace of the original file (in this example name it "file.exe")
3. Use your software as always; expect "interceptor-*.txt" logs on Desktop

You can override logs dir and file name by environment variables `PARAMS_INTERCEPTOR_LOGS_DIR` and `PARAMS_INTERCEPTOR_LOGS_NAME`.

---
Features list:
- [x] Intercepting & logging
  - [x] Parameters
  - [x] Working dir
  - [x] Env. variables
- [ ] Configuration
  - [x] Logs location
  - [ ] Auto-modify params passed
    - [ ] Add
    - [ ] Replace
    - [ ] Remove
