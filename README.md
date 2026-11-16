# Omni

Omni 是 UE 5.8 C++ 项目。运行时由 `OmniGame` 组装 Experience、角色和界面；编辑器扩展由 `OmniEditor` 提供。默认启动地图为 `/Game/L_GameStart`，具体玩法与演示由 `Plugins/GameFeatures` 管理。

## 打开和构建

本目录同时是 Git 根目录和 UE 工程根目录。克隆包含 LFS 资产的分支时，先安装 Git LFS；如果工作区只有指针文件，在本目录运行 `git lfs install --local` 和 `git lfs pull`。然后使用匹配的 UE 5.8 打开 [Omni.uproject](Omni.uproject)。

引擎关联为 `5.8`，本地工程基线为 UE `5.8.4`（CL `58902052`）。需要编译时，使用该引擎的 `Engine/Build/BatchFiles/Build.bat`，传入 `OmniEditor Win64 Development` 和 `-Project=<Omni.uproject 的绝对路径>`。游戏、客户端、服务器目标分别为 `OmniGame`、`OmniClient`、`OmniServer`，构建配置以对应的 [Target 文件](Source/) 为准。

## 目录

| 目录 | 职责 |
| --- | --- |
| `Config/`、`Content/` | Omni 的配置与项目资产 |
| `Source/OmniGame/` | 运行时逻辑与项目集成 |
| `Source/OmniEditor/` | 编辑器扩展，例如角色导入和资产工具 |
| `Plugins/Custom/` | [可复用框架、玩法系统与共享内容](Plugins/Custom/README.md) |
| `Plugins/GameFeatures/` | 具体玩法与演示 |
| `Docs/` | 随仓库维护的团队文档，例如 [C++ 编码规范](Docs/CppCodingStyle.md) |

`Plugins/Custom` 的源码和资源随 Omni 仓库维护。当前插件描述符的 `VersionName` 为 `1.0`，实际版本以随项目检出的 Git 提交为准；实现参考基线为 Lyra 5.8。

本地工作区中的 `Misc/` 和 `OmniAsset/` 位于仓库外，分别用于辅助产物和共享源资源。分支、工具归属和检查方式见 [协作规范](AGENTS.md)。
