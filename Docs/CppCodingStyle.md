# Omni C++ 编码规范

适用于本项目新增和修改的 C++、`Build.cs` 和 `Target.cs`。第三方代码保留原有约定和版权；既有代码随相关功能修改逐步统一。

## 命名和格式

- 使用 PascalCase 和 UE 类型前缀（`U`、`A`、`F`、`E`、`I`、`S` 等）；布尔变量以 `b` 开头。用 `Find`、`Can`、`Try` 等动词表达接口用途和结果。
- 公共接口放 `Public`，实现及内部头文件放 `Private`，按职责组织。类先列公共接口，再列扩展点和内部实现。
- 缩进使用 Tab、显示宽度 4，指针和引用符号贴近类型，例如 `UObject* Object`、`const FItemData& Data`。类型、函数和控制流的大括号独占一行。
- 源码使用 UTF-8 和 LF，文件末尾保留换行；基础格式以仓库 `.editorconfig` 为准。新增反射声明使用紧凑的说明符，例如 `Category="Omni|Input"`、`meta=(AllowPrivateAccess="true")`。

## 注释与依赖

- 中文 `/** ... */` 写在声明及反射宏之前，说明职责、约束或返回结果。函数内部用 `//` 解释实现原因、顺序和边界。
- 头文件使用 `#pragma once`，直接包含所用依赖，可合法前置声明的类型按需前置声明。`.generated.h` 是头文件最后一条 include；`.cpp` 首先包含对应头文件。
- 公共头文件对消费者公开的模块依赖放 `PublicDependencyModuleNames`，其余放 `PrivateDependencyModuleNames`。运行时模块不依赖编辑器模块，插件遵守 [Custom 分层](../Plugins/Custom/README.md) 并声明实际依赖。
- 按跨模块使用决定导出范围，使用所属模块的 API 宏。已有 `MinimalAPI` / `UE_API` 类型沿用原策略，头文件末尾 `#undef UE_API`。按真实装配和调用场景开放蓝图访问及编辑权限。

## 状态与生命周期

- 初始化成员默认值，只读接口标 `const`，重写显式标 `override`。共享配置与对象运行状态分离。
- 需要 GC 跟踪的对象成员使用 `UPROPERTY` 与 `TObjectPtr`；非持有的长期对象引用用 `TWeakObjectPtr`，按需加载的资产用软引用。局部借用对象可用裸指针。
- 对可能失败的调用说明状态是否已修改。`check` 用于程序不变量，`ensure` 用于需要诊断但可以恢复的情况；玩家可触发的正常失败通过结果返回。
- 在对应生命周期内成对注册和清理输入、能力、UI 与委托。日志使用所属系统分类，并提供能定位问题的上下文。

## 提交前检查

只对本次修改范围做 Rider 格式化和适用诊断，检查接口、注释和模块归属。按改动选择必要的编译或运行验证；发现具体问题再追加针对性检查。项目的构建目标与验证约定见 [AGENTS.md](../AGENTS.md#构建与验证)。
