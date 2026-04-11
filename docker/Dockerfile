# syntax=docker/dockerfile:1.7

# ==================== 第一阶段：构建阶段 ====================
FROM ubuntu:24.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

# 安装基础构建工具 + CMake（推荐方式）
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

# === 使用 Kitware 官方源安装较新 CMake（推荐）===
RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null && \
    echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ noble main' | tee /etc/apt/sources.list.d/kitware.list >/dev/null && \
    apt-get update && \
    apt-get install -y --no-install-recommends cmake && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /opt
RUN git clone --depth=1 https://github.com/microsoft/vcpkg.git && \
    /opt/vcpkg/bootstrap-vcpkg.sh -disableMetrics && \
    rm -rf /opt/vcpkg/buildtrees /opt/vcpkg/downloads /opt/vcpkg/.git

WORKDIR /app
COPY . .

# 先运行 vcpkg install，利用缓存
RUN /opt/vcpkg/vcpkg install --feature-flags=manifests

# 编译项目
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