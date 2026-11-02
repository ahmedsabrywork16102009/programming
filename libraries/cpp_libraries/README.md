# 📚 Ahmed Sabry (AS) C++ Libraries

مكتبات C++ قياسية ومنظمة بنمط **Header-Only** وتعمل مع كافة معايير C++ (C++17 / C++20 / C++23).

---

## 🗂️ هيكل المجلدات

```text
cpp_libraries/
├── as.h                  # 🌟 المجمع الرئيسي لكافة المكتبات (يكفي استدعاؤه وحده)
├── core/                 # 1. الأساسيات (بدون أي تبعيات I/O أو ملفات)
│   ├── math.h            # الرياضيات، الأرقام العشوائية (mt19937)، والتحقق
│   ├── string.h          # معالجة النصوص، التقطيع (split)، والـ trim
│   └── datetime.h        # هياكل stDate و stPeriod وحسابات التقويم
├── io/                   # 2. الإدخال والإخراج عبر الطرفية
│   ├── input.h           # قراءة والتحقق من النصوص، الأرقام، والتواريخ
│   └── output.h          # طباعة الجداول، التقاويم، وإدارة الشاشة
├── storage/              # 3. حفظ واسترجاع الملفات
│   └── file_utils.h      # دوال عامة لقراءة وكتابة الأسطر والملفات
└── domain/               # 4. نماذج بيانات خاصة بالمشاريع
    └── clients.h         # هيكل stClientData وعمليات بنك العملاء
```

---

## 🚀 طريقة الاستخدام السريعة

في أي مشروع أو ملف، يكفيك سطرين فقط للوصول لكافة الدوال والهياكل:

```cpp
#include "as.h"

using namespace std;
using namespace AS;

int main() {
    stDate today = getSystemDate();
    printDateDetails(today, "Today is: ");
    return 0;
}
```

