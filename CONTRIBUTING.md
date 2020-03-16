# 为GVDS贡献代码 
首先，多谢您为项目做出贡献！

下面是开发以及贡献代码时的一些注意事项，如果您对这些注意事项有**更好的意见**，欢迎提出`Merge Request`以修改本文档。



Table of Contents

[TOC]

## 代码风格

出于统一代码风格的目的，我们使用**Google 代码风格** ，可以参阅[Google开源项目风格指南](https://zh-google-styleguide.readthedocs.io/en/latest/google-cpp-styleguide/)。不过为了开发速度，我们**不会严格要求**代码严格遵守该指南。

目前主流的IDE基本都提供了自动排版工具，例如Visual Studio Code，您可以在`File -> Preferences -> Settings -> User Settings-> Extensions -> C/C++ -> Clang_format_fallback Style  `中填写`Google`，然后选中代码，使用快捷键`Ctrl + Shift + I`来自动排版。



## 命名指南

由于老师们目前还**没有决定名字**，因此项目暂时使用`gvds`作为前缀。为了防止与外部依赖库冲突，最好将您的类和函数包裹到`namespace gvds`中。



## 开发环境

### 操作系统及依赖库

我们对于代码编辑器以及操作系统没有特殊要求，但是我们要求您编写的代码能够在主流Linux平台、主流Linux版本上编译运行，包括但不限于

- Linux kernel 2.6, Linux Kernel 3.10, Linux kernel 4.15 等各个版本Linux
- Ubuntu 14,16,18 ，CentOS 6, 7

我们依赖的软件版本如下：

- GCC >= 7.3
- CMake >= 3.12
- libgtest-dev >= 1.8.0

**notice**: 为了实现项目，我们还会持续不断地引入新的外部框架或工具。这些工具可能由于版本差异较大等原因，不能通过apt或yum等包管理工具安装，而是需要编译安装。如果该工具的代码量过大，我们不推荐使用`git submodule`的方式将它集成到本项目中。您可以

- 创建一个manifest仓库，将所有外部依赖库和本项目都作为submodule，并且编写统一的编译脚本

- 创建或修改脚本install-env.sh，在脚本中下载所需版本的源代码包或`rpm, deb`包，并编译安装
- 将您依赖的外部库以及对应的版本，添加到本文档的[操作系统及依赖库](#操作系统及依赖库)



### 开发语言

- c++ 17: 出于加速开发的目的，我们决定在本项目中使用部分C++ 17的特性，您可以使用**C++ 17**中GCC 7.3所支持的任意语法。
- Python 3: 由于Python 2即将停止维护，我们希望您使用Python 3作为候选开发语言。您可以自由选择Python 3来开发耦合度较低的代码，比如构建、测试脚本，工具脚本，Web服务等
- Java: **本项目暂时没有使用Java的计划**，如果您希望使用Java，可以修改本段落
- Web 后端语言：**本项目暂时未决定使用何种语言及框架来实现Web后端**，我们欢迎您提出建议以及理由，例如：`我建议使用python-django，因为简单而且会的人多`， `我建议选择nodejs，因为我想用一下`， `我建议选择php，因为...`
- Web前端框架：*未定*



## 目录划分

- src: 源代码目录，包括头文件、公共组件以及各模块主逻辑代码
- doc: 文档目录，可以使用markdown或者rst语法，doxygen会自动编译文档
- tests: 测试代码目录，您应该为您的组件编写测试用例，并且注册到CMakeFile中



## 讨论与贡献

我们建议在gitlab的  [ISSUE](https://gitlab.com/buaaica/hvs-one/issues/new)  中进行讨论，这样讨论的过程所有成员都能参与，并且能够更好的记录。

