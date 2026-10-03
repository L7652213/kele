# QQ 飞车道具查询器    QQ飞车交流群：782834246

这是一个可直接用 Visual Studio 2022 打开的原生 Win32 C++ 工程。

打开 `qq-feiche-item-viewer.sln`，选择 `x64` 和 `Debug` 或 `Release` 后生成即可。项目使用 Unicode、UTF-8 源码、WinHTTP、GDI+ 和 Common Controls；数据仍来自 `https://www.bnnb.cn/api.php`，图片来自 QQ 官方图床。

界面采用可乐红、奶油白和深红按钮配色。所有 Windows 控件使用宽字符 API，JSON 响应按 UTF-8 转换为 UTF-16，避免中文乱码。

列表每页固定加载 60 条，翻页时替换当前页数据；窗口支持缩放，列表列宽会随客户区自动调整。图片优先使用 QQ 官方图床，失败时改用站点 `img.php`，尚未下载完成时显示可乐风格占位图。

网络访问由系统 WinHTTP 完成。如果目标接口或图床在当前网络不可达，程序会保留列表和缓存并显示空结果，不会影响工程编译。
