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
    python3-pip \
    wget \
    gnupg \
    && rm -rf /var/lib/apt/lists/*

# 使用 Kitware 官方源安装较新 CMake
RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null && \
    echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ noble main' | tee /etc/apt/sources.list.d/kitware.list >/dev/null && \
    apt-get update && \
    apt-get install -y --no-install-recommends cmake && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /opt

# 关键修复：不要用 --depth=1，全量克隆 vcpkg（解决 git tree read-tree 问题）
RUN git clone https://github.com/microsoft/vcpkg.git && \
    /opt/vcpkg/bootstrap-vcpkg.sh -disableMetrics && \
    # 清理不必要的文件，减小 builder 体积
    rm -rf /opt/vcpkg/buildtrees /opt/vcpkg/downloads /opt/vcpkg/.git

WORKDIR /app

# 重要优化：先复制 vcpkg 清单文件，利用 Docker 缓存（vcpkg install 只在依赖变化时重新跑）
COPY vcpkg.json vcpkg-configuration.json ./

RUN /opt/vcpkg/vcpkg install --feature-flags=manifests

# 再复制整个项目源码
COPY . .

RUN cmake -S . -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DVCPKG_FEATURE_FLAGS=manifests && \
    cmake --build build --config Release -j"$(nproc)"

# ==================== 第二阶段：运行阶段 ====================
FROM ubuntu:24.04 AS runtime

RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /app/build/sl-service /app/sl-service

EXPOSE 4399

CMD ["/app/sl-service"]