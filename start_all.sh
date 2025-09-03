#!/bin/bash

echo "🚀 启动分布式搜索引擎系统..."

# 检查必要的服务
echo "📋 检查环境依赖..."

# 检查MySQL是否运行
if ! pgrep -x "mysqld" > /dev/null; then
    echo "❌ MySQL服务未运行，请先启动MySQL"
    exit 1
fi

# 检查Redis是否运行
if ! pgrep -x "redis-server" > /dev/null; then
    echo "❌ Redis服务未运行，请先启动Redis"
    exit 1
fi

echo "✅ 环境检查通过"

# 编译程序
echo "🔨 编译程序..."

echo "编译爬虫程序..."
./run_v3.sh
if [ $? -ne 0 ]; then
    echo "❌ 爬虫程序编译失败"
    exit 1
fi

echo "编译搜索程序..."
./run_API.sh
if [ $? -ne 0 ]; then
    echo "❌ 搜索程序编译失败"
    exit 1
fi

echo "✅ 编译完成"

# 启动服务
echo "🚀 启动服务..."

# 启动爬虫worker (后台运行)
echo "启动爬虫worker..."
./workernew &
CRAWLER_PID=$!
echo "爬虫进程ID: $CRAWLER_PID"

# 等待一下让爬虫初始化
sleep 2

# 启动Flask API服务 (后台运行)
echo "启动Flask API服务..."
python3 test.py &
API_PID=$!
echo "API进程ID: $API_PID"

# 等待API服务启动
sleep 3

# 启动前端Web服务 (后台运行)
echo "启动前端Web服务..."
python3 -m http.server 8080 &
WEB_PID=$!
echo "Web服务进程ID: $WEB_PID"

# 等待所有服务启动
sleep 2

echo ""
echo "🎉 所有服务启动完成！"
echo ""
echo "📱 访问地址:"
echo "   - 前端界面: http://localhost:8080"
echo "   - API接口:  http://localhost:5001/search?q=查询词"
echo "   - 命令行:   ./out '查询词'"
echo ""
echo "🔄 服务状态:"
echo "   - 爬虫Worker: $CRAWLER_PID"
echo "   - Flask API:  $API_PID"
echo "   - Web服务:   $WEB_PID"
echo ""
echo "⏹️  停止所有服务: kill $CRAWLER_PID $API_PID $WEB_PID"
echo ""

# 保存进程ID到文件，方便后续停止
echo "$CRAWLER_PID $API_PID $WEB_PID" > .pids

# 等待用户输入
read -p "按回车键停止所有服务..."
echo "🛑 正在停止服务..."

# 停止所有服务
kill $CRAWLER_PID $API_PID $WEB_PID 2>/dev/null
rm -f .pids

echo "✅ 所有服务已停止"
