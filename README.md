OWenT’s Utils -- C&CPP
=============

构建环境     | Linux (GCC & Clang) | Windows |
-------------|---------------------|---------|
当前构建状态 | [![Build Status](https://travis-ci.org/owent-utils/c-cpp.svg?branch=master)](https://travis-ci.org/owent-utils/c-cpp) | [![Build status](https://ci.appveyor.com/api/projects/status/tcei0gwqc52veok5?svg=true)](https://ci.appveyor.com/project/owt5008137/c-cpp) |

### 文件夹说明
**include**  -- 头文件

**src**      -- 生成库的代码文件

**sample**   -- 部分模块的代码使用示例

**test**     -- 部分模块的单元测试（包含了gtest源码）

##### CMakeLists.txt 仅针对GCC编写(特别是编译选项部分), VC的话包含include文件夹，添加src下的所有文件即可 (* ^ _ ^ *)
###### 注: MSYS下，生成gtest不正常，所以单元测试无法生成，Cygwin下正常

**注：某些特定分类的算法记录和说明在include/[特定分类名称]目录的README.md中**