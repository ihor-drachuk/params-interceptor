# Params interceptor

Shows with what parameters the application is launched.

Instructions:
1. Rename target (studied) exe file: "file.exe" -> "file.exe_orig"
2. Download [Interceptor](https://github.com/ihor-drachuk/params-interceptor/releases/download/v0.2/interceptor_v0.2.exe) and put it inplace of the original file (in this example name it "file.exe")
3. Use your software as always; expect "interceptor-*.txt" logs on Desktop

You can override logs dir and file name by environment variables `PARAMS_INTERCEPTOR_LOGS_DIR` and `PARAMS_INTERCEPTOR_LOGS_NAME`.

---
### Configuration

The interceptor can be configured via environment variables:

| Variable | Description | Default |
|----------|-------------|---------|
| `PARAMS_INTERCEPTOR_LOGS_DIR` | Override logs directory | Desktop |
| `PARAMS_INTERCEPTOR_LOGS_NAME` | Override logs filename | `interceptor-{app}-{timestamp}-{id}.txt` |
| `PARAMS_INTERCEPTOR_LOG_PARAMS` | Set to `0` to disable logging parameters | Enabled |
| `PARAMS_INTERCEPTOR_LOG_WORKDIR` | Set to `0` to disable logging working directory | Enabled |
| `PARAMS_INTERCEPTOR_LOG_ENV` | Set to `0` to disable logging environment variables | Enabled |
| `PARAMS_INTERCEPTOR_LOG_FILES` | Set to `1` to log content of files from params | Disabled |
| `PARAMS_INTERCEPTOR_ADD_PARAMS` | Parameters to add (separated by `\|`) | - |
| `PARAMS_INTERCEPTOR_REPLACE_PARAMS` | Parameters to replace (format: `old1=new1\|old2=new2`) | - |
| `PARAMS_INTERCEPTOR_REMOVE_PARAMS` | Parameters to remove (separated by `\|`) | - |

---
### Features list & future plans:
- [x] Intercepting & logging
  - [x] Parameters
  - [x] Working dir
  - [x] Env. variables
  - [x] Unicode support
- [x] Configuration
  - [x] Logs location
  - [x] Better handling of multiple instances
  - [x] Selection of what to log
  - [x] Auto-modify params passed
    - [x] Add
    - [x] Replace
    - [x] Remove
  - [x] Auto-handle data in params
    - [x] Log content of files which are specified by parameters
