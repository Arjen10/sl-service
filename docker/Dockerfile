# syntax=docker/dockerfile:1.7

# ==================== 第一阶段：构建阶段 ====================
FROM ubuntu:24.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    git \
    curl \
    zip \
    unzip \
    tar \
    pkg-config \
    ninja-build \
    ca-certificates \
    python3 \
    python3-pip && \
    rm -rf /var/lib/apt/lists/*

# 安装较新版本的 CMake（vcpkg 推荐较新版本）
RUN python3 -m pip install --no-cache-dir "cmake>=3.30"

WORKDIR /opt
RUN git clone --depth=1 https://github.com/microsoft/vcpkg.git && \
    /opt/vcpkg/bootstrap-vcpkg.sh -disableMetrics && \
    # 清理 vcpkg 不必要的文件，大幅减小 builder 镜像体积
    rm -rf /opt/vcpkg/buildtrees /opt/vcpkg/downloads /opt/vcpkg/.git

WORKDIR /app
COPY . .

# 先单独运行 vcpkg install，利用 Docker layer 缓存（强烈推荐）
RUN /opt/vcpkg/vcpkg install --feature-flags=manifests

# 配置并编译（Release 模式，多核加速）
RUN cmake -S . -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DVCPKG_FEATURE_FLAGS=manifests && \
    cmake --build build --config Release -j"$(nproc)"

# ==================== 第二阶段：运行阶段（使用 Debian 12） ====================
FROM ubuntu:24.04 AS runtime

# 只安装最少的运行时依赖
# ca-certificates 通常是必须的（如果你的服务需要 HTTPS/网络请求）
# libstdc++6 是大多数 C++ 程序的必需运行时库
RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# 只复制编译好的可执行文件
COPY --from=builder /app/build/sl-service /app/sl-service

# 如果你的程序还需要其他运行时资源（配置文件、证书、数据文件等），在这里复制
# COPY --from=builder /app/config /app/config

EXPOSE 4399

CMD ["/app/sl-service"]