# مكتبات Ahmed Sabry لـ C++

المكتبات **Header-Only** ومقسمة حسب المسؤولية. استخدم أصغر header تحتاجه، واستدعِ الدوال باسم المكتبة مثل `bank::` و`storage::` بدون `using namespace`.

## هيكل المجلدات بالتفصيل

```text
programming/
└── libraries/
    └── cpp_libraries/
        ├── README.md
        ├── as.h
        ├── enumeration.h
        ├── process.h
        │
        ├── core/
        │   ├── math.h
        │   ├── string.h
        │   └── datetime.h
        │
        ├── io/
        │   ├── input.h
        │   └── output.h
        │
        ├── storage/
        │   └── file_utils.h
        │
        └── domain/
            ├── bank.h
            └── clients.h
```

### المسار الأساسي

جذر المكتبات الفعلي هو:

```text
programming/libraries/cpp_libraries/
```

أما مجلد `libraries` فليس داخل مشروع البنك؛ لذلك المسار التالي غير صحيح:

```text
projects/cpp_projects/bank/libraries/cpp_libraries/
```

ومن مجلد مشروع البنك إلى المكتبات يكون المسار النسبي:

```text
../../../libraries/cpp_libraries/
```

عند ضبط IntelliSense أو المترجم، أضف مجلد `cpp_libraries` نفسه إلى include path. عندها يمكن كتابة:

```cpp
#include "domain/bank.h"
```

بدل كتابة المسار النسبي الطويل في كل ملف.

## 1. ملفات الجذر ووحدات المكتبة

الجذر يحتوي المجمع الاختياري وبعض واجهات المكتبة العامة. وحدات الإدخال والإخراج والعملاء موجودة مرة واحدة فقط داخل مجلداتها المتخصصة.

#### `as.h` — مجمّع المكتبات

يضم الوحدات ويوفر aliases منظمة مثل `AS::bank::` و`AS::storage::`.

استخدمه عندما تريد تضمين معظم المكتبات من نقطة دخول واحدة. وهو اختياري.

#### `enumeration.h` — التعدادات

يعرّف enums العامة الخاصة بالأعداد والحروف والأشهر والأيام والعمليات وتنسيقات التواريخ.

مساحة الاسم: `enumeration::`

#### `process.h` — واجهة المعالجة التوافقية

يجمع `enumeration.h` وأدوات `core`، ويسهّل استخدام المشاريع القديمة لهذه الأدوات.

مساحة أسماء أدوات المعالجة: `process::`

#### وحدات الإدخال والإخراج

هذه هي الملفات الفعلية، وليست لها نسخ مكررة في الجذر:

- `io/input.h`، مساحة الاسم `input::`.
- `io/output.h`، مساحة الاسم `output::`.

#### `domain/clients.h` — وحدة العملاء

يحتوي التنفيذ الفعلي للعملاء، ومساحة الاسم المستخدمة هي `clients::`. لا توجد نسخة أخرى منه في الجذر.

#### `README.md` — دليل المكتبة

يوضح بنية المكتبات، اتجاه اعتماد الوحدات، وكيفية اختيار المكان المناسب لأي إضافة.

### الفرق بين `as.h` و`process.h`

- `as.h` يضم الوحدات المختلفة كلها، بما فيها `bank` و`clients`، ويوفر مسارات واضحة مثل `AS::bank::` و`AS::input::`.
- `process.h` يجمع أدوات المعالجة الأساسية، وهو موجود لتسهيل استخدام بعض المشاريع القديمة؛ ليس بديلًا عن `as.h`.
- لا يستورد أي منهما أسماء الدوال بشكل مسطح، ولا تحتاج إلى `using namespace`.

## 2. `core/` — الأدوات الأساسية

تحتوي على وظائف عامة يمكن استخدامها دون واجهة مستخدم أو ملفات أو منطق بنك.

#### `core/math.h` — الرياضيات

يحتوي عمليات وفحوصات حسابية عامة، مثل فحص نوع العدد واستخدام عمليات النطاقات.

#### `core/string.h` — النصوص

يحتوي أدوات معالجة النصوص مثل التقسيم والتغيير وإزالة المسافات.

مثال: `process::splitByString(...)`

#### `core/datetime.h` — التاريخ والوقت

يحتوي أنواع التاريخ والفترة وحسابات التقويم والتواريخ.

أمثلة: `process::stDate`، حساب اليوم، أو إيجاد الفرق بين تاريخين.

دوال هذه الملفات ضمن `process::`. هذه الطبقة هي الأساس الذي يمكن أن تعتمد عليه طبقات أعلى، ولا ينبغي أن تعتمد على `domain/` أو على شاشات البنك.

## 3. `io/` — الإدخال والإخراج

هذه الطبقة مسؤولة عن التفاعل مع الطرفية:

#### `io/input.h` — قراءة الإدخال

يقرأ النصوص والأرقام والاختيارات والتواريخ، ويتحقق من صحة الإدخال.

أمثلة: `input::readString(...)` و`input::readNumber<int>(...)`

#### `io/output.h` — عرض النتائج

يطبع النصوص والجداول والتواريخ، ويدير إعادة تهيئة الشاشة.

أمثلة: `output::printDateDetails(...)` و`output::resetScreen(...)`

قد تعتمد `input` على `output` أو `core`، لكن لا تضع فتح الملفات أو تحويل بيانات البنك داخل `io/`.

## 4. `storage/` — إدارة الملفات

`storage/file_utils.h` هو المكان الموحد للتعامل العام مع الملفات النصية. هذه الطبقة تعرف الأسطر (`std::string`) فقط، ولا تعرف ما هو عميل أو مستخدم أو حساب بنكي.

#### `storage::loadLinesFromFileChecked(...)`

يحمّل الأسطر من الملف إلى `std::vector<std::string>`.

#### `storage::writeLinesToFileChecked(...)`

يكتب مجموعة أسطر إلى الملف.

#### `storage::appendLineToFileChecked(...)`

يضيف سطرًا في نهاية الملف.

#### `storage::stFileResult`

نتيجة العملية؛ يحتوي `succeeded` للنجاح و`message` لتفاصيل الخطأ.

#### `storage::reportFileError(...)`

يطبع تفاصيل الخطأ إلى `std::cerr`.

مثال قراءة مع فحص النتيجة:

```cpp
#include "storage/file_utils.h"

std::vector<std::string> vLines;
const storage::stFileResult fileResult =
    storage::loadLinesFromFileChecked("files/Clients.txt", vLines);

if (!fileResult.succeeded) {
    storage::reportFileError(fileResult);
}
```

`loadLinesFromFileChecked` يتجاهل الأسطر الفارغة افتراضيًا. مرّر `true` كوسيط ثالث للاحتفاظ بها:

```cpp
storage::loadLinesFromFileChecked("files/Clients.txt", vLines, true);
```

الدوال القديمة `loadLinesFromFile` و`writeLinesToFile` و`appendLineToFile` ما زالت متاحة للتوافق. عند فشلها تطبع الخطأ إلى `std::cerr`؛ في الكود الجديد يفضل استخدام الدوال المنتهية بـ`Checked` وفحص `succeeded`.

## 5. `domain/` — بيانات وقواعد المجال

تحتوي هذه الطبقة أنواع البيانات والعمليات التي تفهم معنى السجلات. هي المكان المناسب لربط سجل العميل بقواعد البنك، لكنها لا ينبغي أن تعيد كتابة قارئ ملفات عام موجود في `storage/`.

#### `domain/clients.h` — نطاق العملاء

يحتوي نوع العميل وقراءة بياناته وتعديلها وتحويله بين سجل وسطر وعرضه. يستخدم `storage` للحفظ والتحميل.

مساحة الاسم: `clients::`

#### `domain/bank.h` — نطاق البنك

يحتوي أنواع البنك `stClientData` و`stUser`، والتحويل والبحث وعمليات الرصيد وعمليات CRUD المشتركة بين برنامج البنك وATM. يستخدم `storage` للملفات.

مساحة الاسم: `bank::`

النوعان `clients::stClientData` و`bank::stClientData` تابعان لوحدتين مختلفتين. لا تخلط بينهما؛ اختر النوع الذي تستخدمه بقية واجهة مشروعك.

مثال استدعاء مباشر:

```cpp
#include "domain/bank.h"

std::vector<bank::stClientData> vClients =
    bank::loadClientDataFromFile("files/Clients.txt");
```

## 6. اتجاه الاعتماد بين الطبقات

اتجاه السهم هنا يعني أن الملف على اليسار يعتمد على الملف على اليمين:

```text
process.h
  ├── enumeration.h
  └── core/{math,string,datetime}.h

io/input.h
  ├── core/datetime.h
  ├── enumeration.h
  └── io/output.h

io/output.h
  ├── core/{datetime,math}.h
  └── enumeration.h

storage/file_utils.h
  └── مستقل؛ يتعامل مع مسارات وأسطر نصية فقط

domain/clients.h
  ├── core/string.h + enumeration.h
  ├── io/{input,output}.h
  └── storage/file_utils.h

domain/bank.h
  ├── io/input.h + io/output.h + process.h
  └── storage/file_utils.h

as.h
  └── يجمع الوحدات السابقة ويعرض aliases اختيارية لها
```

قواعد الاعتماد:

1. `core/` لا يعتمد على `io/` أو `storage/` أو `domain/`.
2. `io/` يستخدم الأدوات الأساسية عند الحاجة، ويبقى مسؤولًا عن الطرفية.
3. `storage/` مستقل عن أنواع البيانات؛ يأخذ مسار ملف وأسطرًا نصية ويعيد نتيجة.
4. `domain/` يستخدم `storage/` للحفظ والتحميل، ويحول الأسطر إلى أنواع المجال والعكس.
5. `as.h` يجمع الوحدات للاستخدام الاختياري، لكنه لا يدمج أسماءها في مساحة واحدة.

## 7. أضع ملفًا أو وظيفة جديدة فين؟

- خوارزمية عامة لا تخص مجالًا بعينه؟ ضعها في `core/`.
- قراءة أو طباعة تفاعلية؟ ضعها في `io/`.
- فتح أو قراءة أو كتابة ملف بغض النظر عن محتواه؟ ضعها في `storage/`.
- نوع بيانات أو قواعد خاصة بالعملاء أو البنك؟ ضعها في `domain/`.
- ملف يخص وحدة موجودة؟ ضعه في مجلدها المتخصص، ولا تنشئ wrapper مكررًا في الجذر.

عند إضافة وحدة جديدة:

1. اختر الطبقة المناسبة أولًا.
2. أنشئ التنفيذ داخل المجلد المسؤول.
3. استخدم namespace باسم الوحدة، مثل `storage::` أو `bank::`.
4. لا تضف `using namespace` في header.
5. أضف الوحدة إلى `as.h` فقط إذا كانت عامة ومناسبة للمجمّع.
6. حدّث هذا الدليل واختبر المشاريع التي تستخدم الوحدة.

## الاستخدام

أضف `libraries/cpp_libraries` إلى مسار تضمين المترجم وIntelliSense، ثم ضمّن أصغر وحدة تحتاجها:

```cpp
#include "domain/bank.h"
#include "storage/file_utils.h"
```

أو استخدم المجمع الاختياري مع إبقاء أسماء الوحدات واضحة:

```cpp
#include "as.h"

auto vClients = AS::bank::loadClientDataFromFile("files/Clients.txt");
auto today = AS::process::getSystemDate();
AS::output::printDateDetails(today, "Today is: ");
```

استخدم بادئة المكتبة (`bank::` أو `storage::` أو `process::`) بدلًا من `using namespace`.
