# MikoView Framework

MikoView เป็นเฟรมเวิร์กสำหรับสร้างแอปพลิเคชันเดสก์ท็อปที่ทันสมัย ที่ผสมผสานพลังของ CEF (Chromium Embedded Framework) กับ SDL เพื่อสร้างแอปพลิเคชันเดสก์ท็อปข้ามแพลตฟอร์มโดยใช้เทคโนโลยีเว็บ

## คุณสมบัติ

- 🚀 **เทคโนโลยีเว็บสมัยใหม่**: สร้างแอปเดสก์ท็อปด้วย HTML, CSS และ JavaScript/TypeScript
- 🎯 **ข้ามแพลตฟอร์ม**: รองรับ Windows และ Linux
- ⚡ **ประสิทธิภาพสูง**: ประสิทธิภาพระดับ Native ด้วยการรวม CEF และ SDL
- 🎨 **รูปลักษณ์ Native**: การจัดธีมเฉพาะแพลตฟอร์มและรองรับโหมดมืด
- 📁 **การเข้าถึงระบบไฟล์**: การดำเนินการระบบไฟล์ที่ปลอดภัย
- 🔧 **รองรับ TypeScript**: คำจำกัดความ API แบบ TypeScript ครบถ้วน
- 🎮 **การรวม SDL**: การเรนเดอร์แบบเร่งด้วยฮาร์ดแวร์และการจัดการอินพุต

## เริ่มต้นอย่างรวดเร็ว

### ข้อกำหนดเบื้องต้น

- CMake 3.15+
- คอมไพเลอร์ที่รองรับ C++17
- CEF Binary Distribution
- ไลบรารี SDL2 development
- Node.js 16+ (สำหรับการพัฒนา renderer)

### การสร้าง

```bash
# โคลนรีโพซิทอรี
git clone https://github.com/arizkami/mikoview.git
cd mikoview

# สร้างโปรเจค
mkdir build
cd build
cmake ..
cmake --build .
```

### แอปแรกของคุณ

```cpp
#include "mikoview.hpp"

int main() {
    MikoView::Application app;
    
    // เริ่มต้นแอปพลิเคชัน
    if (!app.Initialize()) {
        return -1;
    }
    
    // สร้างหน้าต่าง
    app.CreateWindow({
        .title = "แอป MikoView แรกของฉัน",
        .width = 1200,
        .height = 800,
        .url = "file://app/index.html"
    });
    
    // รันแอปพลิเคชัน
    return app.Run();
}
```

## โครงสร้างเอกสาร

- [เริ่มต้นใช้งาน](getting-started.md)
- [คู่มือ API](api-reference.md)
- [ตัวอย่าง](examples.md)
- [การสร้างจากซอร์ส](building.md)
- [การปรับใช้](deployment.md)
- [การมีส่วนร่วม](contributing.md)

## ใบอนุญาต

ใบอนุญาต MIT - ดู [LICENSE](../../LICENSE) สำหรับรายละเอียด