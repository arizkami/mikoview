# เริ่มต้นใช้งาน MikoView

## การติดตั้ง

### จากซอร์สโค้ด

1. **โคลนรีโพซิทอรี**
   ```bash
   git clone https://github.com/arizkami/mikoview.git

   cd mikoview
   ```

2. **ติดตั้ง dependencies**
   - ดาวน์โหลด CEF Binary Distribution สำหรับแพลตฟอร์มของคุณ
   - ติดตั้งไลบรารี SDL2 development
   - ติดตั้ง CMake 3.15+

3. **สร้างเฟรมเวิร์ก**
   ```bash
   mkdir build && cd build
   cmake ..
   cmake --build . --config Release
   ```

## โครงสร้างโปรเจค

```
my-mikoview-app/
├── src/
│   └── main.cpp          # จุดเริ่มต้นแอปพลิเคชัน
├── renderer/
│   ├── index.html        # ไฟล์ HTML หลัก
│   ├── style.css         # สไตล์
│   └── script.js         # ตรรกะ JavaScript
├── assets/
│   └── icon.png          # ไอคอนแอปพลิเคชัน
└── CMakeLists.txt        # การกำหนดค่าการสร้าง
```

## แอปพลิเคชันพื้นฐาน

### 1. แอปพลิเคชันหลัก (C++)

```cpp
#include "mikoview.hpp"

class MyApp : public MikoView::Application {
public:
    bool Initialize() override {
        if (!MikoView::Application::Initialize()) {
            return false;
        }
        
        // ลงทะเบียน custom handlers
        RegisterHandler("app.getVersion", [](const std::string& data) {
            return "{\"version\": \"1.0.0\"}";
        });
        
        return true;
    }
    
    void OnWindowReady() override {
        // หน้าต่างพร้อมแล้ว คุณสามารถโต้ตอบกับ renderer ได้
        InvokeRenderer("app.ready", "{}");
    }
};

int main() {
    MyApp app;
    
    if (!app.Initialize()) {
        return -1;
    }
    
    app.CreateWindow({
        .title = "แอป MikoView ของฉัน",
        .width = 1200,
        .height = 800,
        .url = "file://renderer/index.html",
        .resizable = true,
        .darkMode = true
    });
    
    return app.Run();
}
```

### 2. Renderer (HTML/JavaScript)

```html
<!DOCTYPE html>
<html>
<head>
    <title>แอป MikoView ของฉัน</title>
    <link rel="stylesheet" href="style.css">
</head>
<body>
    <div id="app">
        <h1>ยินดีต้อนรับสู่ MikoView</h1>
        <button id="getVersion">ดูเวอร์ชัน</button>
        <p id="version"></p>
    </div>
    
    <script src="script.js"></script>
</body>
</html>
```

```javascript
// script.js
document.addEventListener('DOMContentLoaded', () => {
    const button = document.getElementById('getVersion');
    const versionEl = document.getElementById('version');
    
    button.addEventListener('click', async () => {
        try {
            const result = await mikoview.invoke('app.getVersion');
            const data = JSON.parse(result);
            versionEl.textContent = `เวอร์ชัน: ${data.version}`;
        } catch (error) {
            console.error('ข้อผิดพลาด:', error);
        }
    });
    
    // ฟังอีเวนต์ app ready
    mikoview.on('app.ready', () => {
        console.log('แอปพร้อมแล้ว!');
    });
});
```

## การดำเนินการระบบไฟล์

```javascript
// อ่านไฟล์
const content = await mikoview.fs.readFile('path/to/file.txt');

// เขียนไฟล์
await mikoview.fs.writeFile('path/to/output.txt', 'สวัสดีชาวโลก!');

// อ่านไดเรกทอรี
const entries = await mikoview.fs.readDir('path/to/directory');

// ตรวจสอบว่าไฟล์มีอยู่หรือไม่
const exists = await mikoview.fs.exists('path/to/file.txt');
```

## ขั้นตอนถัดไป

- [คู่มือ API](api-reference.md) - เอกสาร API ที่สมบูรณ์
- [ตัวอย่าง](examples.md) - ตัวอย่างที่ซับซ้อนมากขึ้น
- [การสร้าง](building.md) - การกำหนดค่าการสร้างขั้นสูง
```