# Medusa

Medusa is a `nvim` plugin that enables me to work on projects that use QT Creator for compiling in `nvim`.

Some of the projects I work on (at work) require me to use QT Creator to compile. The toolchain is highly integrated and I'd rather *not* ditch it entirely. Thus, this plugin allows me to use QT Creator as a sort of "backend" for compiling my code while I develop entirely in `nvim`

## Workflow

```mermaid
sequenceDiagram
    actor user as User
    participant lua as Medusa Lua Plugin
    participant qt_ctrl as QT Controller DLL
    participant qt_hook as QT Hook DLL
    participant logger as Log Displayer Executable
    participant qt as QT Creator Process

    user ->> lua: Run application
    lua ->> logger: Creates as "job" in nvim
    lua ->>  qt_ctrl: Run application
    qt_ctrl ->> qt: Switches to application, sends Ctrl+R keystroke, minimizes application using Win32 API
    qt ->> App: Launches app (ninja, cmake, then app)
    qt ->> qt_hook: CreateProcessW watcher triggerd, copies stdout handles
    loop periodically
        qt_hook ->> logger: Sends logs via IPC periodically
        logger -->> lua: Sends logs via job's stdout
        lua --) user: Logs to output buffer
    end
    lua --) user: Parsed logs to nvim quickfix

```


