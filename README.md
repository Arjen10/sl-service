# sl-service

水利报文接收服务 —— 用于接收、解析并处理来自各类水利监测设备或系统的标准/非标准报文数据。

## 简介
sl-service 是一个轻量级、高可用的后端服务，专为水利行业设计，用于实时接收来自水文站、雨量站等设备上报的报文（目前为止仅支持TCP），
并对原始报文进行校验、解析、转发。

## 核心功能

1、内置水利行业常用报文格式解析器（如SL651-2014）。

2、插件式协议解析器，便于新增报文格式支持。

## 技术栈

C++17、boost asio

## 快速开始

### 先决条件

（推荐）安装 vcpkg 或手动安装 vcpkg.json 中的相关第三方库

### 编译

```text
git clone https://github.com/Arjen10/sl-service.git
mkdir build
cd build
cmake ..
cmake --build .
```
