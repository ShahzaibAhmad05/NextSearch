FROM ubuntu:22.04 AS build
RUN apt-get update && apt-get install -y \
    build-essential cmake ninja-build

WORKDIR /src
COPY . .
RUN cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
 && cmake --build build

FROM ubuntu:22.04
RUN apt-get update && apt-get install -y ca-certificates

WORKDIR /app
COPY --from=build /src/build/api_server /app/api_server

EXPOSE 8080
ENV PORT=8080
CMD ["/app/api_server"]