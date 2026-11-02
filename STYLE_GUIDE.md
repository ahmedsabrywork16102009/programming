# 📜 دليل معايير وتنسيق الكود (Ahmed Sabry - AS Style Guide)

هذا الدليل هو المرجع الرسمي والمعياري لكتابة وتنظيم الأكواد والمكتبات والمشاريع الخاصة بـ **أحمد صبري (AS)**.  
يجب على أي مطور أو مساعد ذكاء اصطناعي (AI Assistant) الالتزام التام بهذه المعايير عند إنشاء أو تعديل أي ملف في مساحة العمل.

---

## 🗂️ 1. هيكل مساحة العمل (Workspace Structure)

```text
programming/
├── libraries/
│   └── cpp_libraries/         # مكتبات C++ القابلة لإعادة الاستخدام (Header-only)
│       ├── core/              # الأساسيات المجردة (بدون أي تبعيات)
│       │   ├── math.h         # دوال العمليات الحسابية والأرقام العشوائية
│       │   ├── string.h       # دوال معالجة النصوص والتقطيع والـ trim
│       │   └── datetime.h     # هيكل stDate و stPeriod وحسابات التقويم بـ O(1)
│       ├── io/                # الإدخال والإخراج عبر الكونسول
│       │   ├── input.h        # قراءة والتحقق من الأرقام والتواريخ
│       │   └── output.h       # طباعة الجداول والتقاويم والشاشات
│       ├── storage/           # التعامل مع الملفات وحفظ السجلات
│       │   └── file_utils.h   # دوال عامة لحفظ وقراءة الأسطر والملفات
│       ├── as.h               # المجمّع الشامل لإطار عمل AS (Master Include)
│       ├── enumeration.h      # الـ Enums المشتركة للتوافقية
│       ├── process.h          # Forwarder لـ core/ للتوافق مع المشاريع القديمة
│       ├── input.h / output.h # Forwarders لـ io/ للتوافق مع المشاريع القديمة
│       └── clients.h          # مكتبة عملاء البنك وسجلات الملفات
├── projects/
│   ├── cpp_projects/          # مشاريع C++ (bank, math-game, rock-paper-scissor)
│   ├── python_projects/       # مشاريع Python
│   ├── chrome_extintion/      # إضافات متصفح Chrome (Manifest V3)
│   └── web_projects/          # تطبيقات الويب (HTML/CSS/JS)
├── learn/                     # ملفات التجارب والتعلم (مثل learn_cpp.cpp)
├── STYLE_GUIDE.md             # هذا الدليل المعياري
└── AGENTS.md                  # تعليمات المساعد الذكي الدائمة (AI Rules)
```

---

## 🏷️ 2. قواعد التسمية (Naming Conventions)

نتبع أسلوب **Hungarian Notation المخصص لـ AS** للتمييز السريع بين الأنواع:

| النوع | القاعدة | البادئة (Prefix) | أمثلة |
| :--- | :--- | :--- | :--- |
| **Structs** | PascalCase | `st` | `stDate`, `stPeriod`, `stClientData`, `stRoundInfo`, `stGameInfo` |
| **Enums** | PascalCase | `en` | `enWeekday`, `enMonth`, `enNumber`, `enCharCase`, `enChoice` |
| **Vectors** | camelCase | `v` | `vStrings`, `vTokens`, `vChars`, `vDateString` |
| **Enum Values** | PascalCase أو `e` | - أو `e` | `Odd`, `Even`, `Upper` أو `eDeposit`, `eWithdraw`, `eAddNewClient` |
| **Functions** | camelCase | - | `readString`, `checkNumberType`, `splitByString`, `daysBetween` |
| **Variables** | camelCase | - | `numberOfRounds`, `playerWinTimes`, `accountBalance` |
| **Constants** | UPPER_SNAKE | - | `ROCK_DRAWING`, `PAPER_DRAWING`, `PROCESS_H` |
| **Namespaces** | lowercase | - | `namespace process`, `namespace input`, `namespace output` |
| **Master Namespace**| UPPERCASE | - | `namespace AS` (Ahmed Sabry) |

---

## 🏛️ 3. تصميم مكتبات C++ (Library Design Pattern)

كل مكتبة داخل `libraries/cpp_libraries/` تتبع هذا النمط المعماري بدقة:

1. **Header-Only:** جميع المكتبات عبارة عن ملفات رأسية تضمن سهولة الـ Include والربط دون الحاجة لملفات `.cpp` منفصلة.
2. **Guards:** استخدام `#pragma once` في أول كل ملف.
3. **فصل الإعلانات عن التنفيذ (Declarations then Implementation):**
   - قسم `// --- Declarations ---` في الأعلى.
   - قسم الفواصل الجمالية:
     ```cpp
     // =============================================================
     //  Section Name
     // =============================================================
     ```
   - قسم التنفيذ مع استخدام الكلمة المفتاحية `inline` لتفادي أخطاء Multiple Definition عند تضمين الملف في أكثر من مكان.

### 4. هيكل النيم سبيس الرئيسي (`namespace AS`)

كل مكتبة تعرّف منطقها داخل Namespace مخصص (`process`, `input`, `output`, `clients`)، ثم يُصدَّر كل شيء لـ `namespace AS`:

```cpp
namespace AS {
    using namespace enumeration;
    using namespace process;
    using namespace input;
    using namespace output;
}
```

وفي كود التطبيقات والمشاريع، يُستدعى بالشكل القياسي:
```cpp
#include "../../../libraries/cpp_libraries/input.h"
#include "../../../libraries/cpp_libraries/process.h"

using namespace std;
using namespace AS;
```

---

## ⚡ 5. الموازنة بين أسلوب AS وأداء Modern C++

عند تطوير أو إضافة ميزات جديدة، نحافظ على روح وأسلوب الكود مع تطبيق أفضل ممارسات C++ الحديثة:

1. **الـ Structs محتفظة بهويتها مع إمكانيات حديثة:**
   - يظل اسم الهيكل `stDate` و `stPeriod` كما هو.
   - تزويد الـ Structs بـ Operator Overloading (`<=>`, `==`, `+`, `-`, `++`, `--`, `<<`) لدعم العمليات الرياضية السلسة.
   - توفير Alias نظيف مثل `using Date = stDate;` لمن يفضله.

2. **كفاءة استهلاك الذاكرة (Memory Efficiency):**
   - استخدام `std::string_view` لمدخلات القراءة فقط لتجنب النسخ العشوائي (No Unnecessary Copies).
   - حجز مساحة الـ Strings والـ Vectors مسبقاً (`reserve()`) في دوال التجميع مثل `splitByString` و `replaceAll`.

3. **الأداء الرياضي (Algorithmic Performance):**
   - تفادي الـ `while loops` التكرارية لحساب الفترات والتواريخ. الحسابات تكون بـ $O(1)$ قدر الإمكان (مثل حساب تداخل الفترات).

4. **الأمان (Safety & Modern Standards):**
   - استخدام `std::mt19937` بدل `rand()` الكلاسيكي.
   - حماية دوال التحويل النصي (`convertToDate`) بـ `try/catch` لمنع انهيار البرنامج عند إدخال بيانات غير صحيحة.
   - عدم استخدام `using namespace` أبداً داخل الـ Global Scope لملفات الـ Header.

---

## 🤖 6. ميثاق المساعد الذكي (AI Persona & Assistant Rules)

عندما يطلب المستخدم إنشاء أو تعديل أي كود C++:
1. **احترم الستايل فوراً وبلا تردد:** استخدم `st` للـ Structs و `en` للـ Enums و `v` للـ Vectors.
2. **ضع الكود في `namespace AS`** واجعله متوافقاً مع مكتبات `libraries/cpp_libraries/`.
3. **لا تكسر التوافق الخلفي أبداً:** إذا أعدت كتابة دالة لتسريعها، حافظ على نفس اسمها وتوقيعها القديم لتعمل كل المشاريع القديمة.
4. **قسّم المكتبات بذكاء:** المكتبات الجديدة تُضاف كـ Modular Headers وتُربط عبر `process.h`.

