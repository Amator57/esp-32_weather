# 🐙 Робота з GitHub у PlatformIO (VS Code)

> Практична інструкція для збереження коду, ведення версій та завантаження змін.  
> Операційна система: **Windows**. Середовище: **VS Code + PlatformIO**.

---

## Зміст

1. [Одноразове налаштування (виконується один раз)](#1-одноразове-налаштування)
2. [Щоденна робота — зберегти зміни](#2-щоденна-робота--зберегти-зміни)
3. [Перегляд історії версій](#3-перегляд-історії-версій)
4. [Повернення до попередньої версії](#4-повернення-до-попередньої-версії)
5. [Завантаження змін з GitHub (оновлення)](#5-завантаження-змін-з-github)
6. [Робота з гілками (безпечне тестування)](#6-робота-з-гілками)
7. [Термінал у VS Code — швидка шпаргалка](#7-термінал-у-vs-code--шпаргалка)
8. [Типові помилки та їх вирішення](#8-типові-помилки)

---

## 1. Одноразове налаштування

Ці кроки виконуються **один раз** на новому комп'ютері або для нового проєкту.

### 1.1 Встановлення Git

1. Завантажте Git: https://git-scm.com/download/win
2. Встановіть з налаштуваннями за замовчуванням
3. Перевірте встановлення — відкрийте термінал VS Code (`Ctrl + ~`) та введіть:

```powershell
git --version
# Очікуваний результат: git version 2.x.x.windows.x
```

### 1.2 Налаштування імені та email (один раз на ПК)

```powershell
git config --global user.name "Ваше Ім'я"
git config --global user.email "your@email.com"
```

### 1.3 Перевірка що проєкт вже пов'язаний з GitHub

```powershell
git remote -v
```

Очікуваний результат:
```
origin  https://github.com/Amator57/esp-32_weather.git (fetch)
origin  https://github.com/Amator57/esp-32_weather.git (push)
```

Якщо виводу немає — виконайте:
```powershell
git remote add origin https://github.com/Amator57/esp-32_weather.git
```

### 1.4 Клонування проєкту на новий комп'ютер

Якщо ви переходите на інший ПК і хочете отримати проєкт з GitHub:

```powershell
# Перейдіть у папку де хочете зберегти проєкт (наприклад c:\Projects_PlatformIO\)
cd c:\Projects_PlatformIO

# Клонуйте репозиторій
git clone https://github.com/Amator57/esp-32_weather.git

# Відкрийте папку в VS Code
code esp-32_weather
```

---

## 2. Щоденна робота — зберегти зміни

Це **основний сценарій**, який повторюється щоразу коли ви щось змінили в коді.

### Відкрийте термінал у VS Code

```
Ctrl + ` (тильда)  або  Ctrl + Shift + `
```

Переконайтеся що ви в папці проєкту:
```powershell
cd c:\Projects_PlatformIO\ESP32__Web_weather_v1_23_04_26
```

### Крок 1 — Перевірте що змінилось

```powershell
git status
```

Приклад виводу:
```
On branch main
Changes not staged for commit:
  modified:   src/main.cpp
  modified:   data/settings.html

Untracked files:
  USER_MANUAL.md
```

- 🟡 `modified` — файл змінено
- 🔴 `deleted` — файл видалено
- 🟢 `Untracked files` — новий файл, якого ще немає в Git

### Крок 2 — Додайте файли до коміту

**Варіант А — додати всі змінені файли:**
```powershell
git add .
```

**Варіант Б — додати конкретний файл:**
```powershell
git add src/main.cpp
git add data/settings.html
```

### Крок 3 — Створіть коміт (збережіть версію)

```powershell
git commit -m "Опис змін: що саме ви зробили"
```

> 📝 **Правило гарного опису коміту:**
> - ❌ Погано: `"fix"`, `"changes"`, `"update"`
> - ✅ Добре: `"Виправлено кнопку перезавантаження"`, `"Додано калібрування вологості"`, `"Стабілізація Wi-Fi: вимкнено modem sleep"`

### Крок 4 — Відправте на GitHub

```powershell
git push origin main
```

Якщо потрібен логін — введіть **GitHub username** та **Personal Access Token** (замість пароля).

> 💡 **Як отримати токен:** GitHub → Settings → Developer Settings → Personal access tokens → Generate new token (обрати scope `repo`).

### Весь процес одним блоком (copy-paste)

```powershell
git add .
git commit -m "Опис ваших змін тут"
git push origin main
```

---

## 3. Перегляд історії версій

### Переглянути список комітів (версій)

```powershell
git log --oneline
```

Приклад виводу:
```
8d72f64 WiFi stability, Static IP configuration via Web, and Reboot fix
1e759d1 Checkpoint before configurable Static IP
ab2d357 Checkpoint before WiFi Events and Static IP
cfb1036 Checkpoint before Wi-Fi stability fixes
fcfb40f Fix: Add 5s delay for IP display
c3b02c1 Fix Wi-Fi connection dropping (Watchdog + Disable Modem Sleep)
db5999e Major Optimization: Circular Buffer & Display fixed
```

Кожен рядок — це **окрема версія** з:
- `8d72f64` — унікальний ідентифікатор (хеш)
- `WiFi stability...` — ваш опис змін

### Переглянути що змінилось в конкретному коміті

```powershell
git show 8d72f64
```

### Переглянути різницю між поточним кодом та останнім комітом

```powershell
git diff
```

### Переглянути зміни в конкретному файлі

```powershell
git diff src/main.cpp
```

---

## 4. Повернення до попередньої версії

> ⚠️ **Уважно читайте** — є безпечний та небезпечний спосіб.

### Варіант А — Переглянути стару версію (БЕЗПЕЧНО, тільки перегляд)

```powershell
# Дізнайтесь хеш потрібного коміту
git log --oneline

# Перемкніться на стару версію (режим "перегляду")
git checkout cfb1036

# Повертайтесь до актуальної версії
git checkout main
```

У режимі перегляду ви можете відкрити файли, скопіювати код, але нові коміти не зберігатимуться в `main`.

### Варіант Б — Скасувати останній коміт, але зберегти зміни у файлах

```powershell
git reset --soft HEAD~1
```

Після цього зміни залишаться у файлах, але коміт буде відмінено. Ви можете переробити коміт.

### Варіант В — Повністю скасувати останній коміт І зміни у файлах (НЕБЕЗПЕЧНО)

```powershell
# ⚠️ Незворотня дія! Всі незбережені зміни будуть втрачені.
git reset --hard HEAD~1
```

### Варіант Г — Відновити конкретний файл до версії з останнього коміту

```powershell
# Відновити один файл (наприклад якщо випадково зіпсували)
git checkout HEAD -- src/main.cpp
```

### Варіант Д — Створити новий коміт що скасовує старий (БЕЗПЕЧНО для GitHub)

```powershell
# Скасує конкретний коміт, але збереже всю历torію
git revert cfb1036
```

---

## 5. Завантаження змін з GitHub

Використовується коли:
- Ви редагували файли безпосередньо на GitHub.com
- Ви працюєте на двох комп'ютерах по черзі
- Хтось інший надіслав зміни в репозиторій

### Перевірити чи є нові зміни на сервері

```powershell
git fetch origin
git status
```

### Завантажити та злити зміни з GitHub

```powershell
git pull origin main
```

### Якщо є конфлікти після pull

Якщо той самий рядок файлу змінено і локально, і на GitHub — виникне **конфлікт злиття**. У файлі з'явиться:

```cpp
<<<<<<< HEAD
// Ваш локальний код
int myValue = 10;
=======
// Код з GitHub
int myValue = 20;
>>>>>>> origin/main
```

**Вирішення:**
1. Відкрийте файл у VS Code
2. VS Code підсвітить конфлікт синіми/зеленими кольорами з кнопками:
   - `Accept Current Change` — залишити ваш код
   - `Accept Incoming Change` — взяти код з GitHub
   - `Accept Both Changes` — залишити обидва варіанти
3. Після вибору збережіть файл
4. Виконайте:
```powershell
git add .
git commit -m "Resolved merge conflict"
git push origin main
```

### Якщо GitHub відхиляє push (rejected)

```powershell
# Спочатку завантажте зміни з GitHub, потім відправляйте свої
git pull origin main
git push origin main
```

---

## 6. Робота з гілками

Гілки дозволяють безпечно **тестувати нові функції**, не ламаючи основний робочий код.

### Створення нової гілки для експерименту

```powershell
# Створити нову гілку і одразу переключитись на неї
git checkout -b feature/static-ip
```

Тепер всі ваші зміни та коміти зберігаються у гілці `feature/static-ip`, а `main` залишається чистим.

### Перегляд всіх гілок

```powershell
git branch
# Поточна гілка позначена зірочкою *
```

### Переключення між гілками

```powershell
git checkout main          # повернутись до основної
git checkout feature/static-ip   # перейти до експерименту
```

### Злиття успішного експерименту в main

```powershell
# 1. Переключіться на main
git checkout main

# 2. Злийте вашу гілку
git merge feature/static-ip

# 3. Відправте на GitHub
git push origin main
```

### Видалення гілки після злиття

```powershell
git branch -d feature/static-ip
```

### Рекомендована схема для цього проєкту

```
main          ← стабільний, робочий код (завжди прошивається на пристрій)
  └── feature/wifi-fix      ← тут тестуєте нові функції
  └── feature/new-display   ← ще один незалежний експеримент
```

---

## 7. Термінал у VS Code — Шпаргалка

### Відкрити термінал

| Дія | Комбінація |
|---|---|
| Відкрити/закрити термінал | `Ctrl + ~` |
| Новий термінал | `Ctrl + Shift + ~` |

### Найпотрібніші команди Git

| Команда | Що робить |
|---|---|
| `git status` | Показати змінені файли |
| `git add .` | Додати всі зміни до коміту |
| `git add src/main.cpp` | Додати один файл |
| `git commit -m "опис"` | Зберегти версію локально |
| `git push origin main` | Відправити на GitHub |
| `git pull origin main` | Завантажити зміни з GitHub |
| `git log --oneline` | Переглянути список версій |
| `git diff` | Переглянути що змінилось |
| `git checkout -- .` | Скасувати всі незбережені зміни ⚠️ |
| `git stash` | Тимчасово заховати зміни |
| `git stash pop` | Повернути заховані зміни |

### Корисне: проігнорувати файли

Деякі файли **не треба зберігати на GitHub** (скомпільований код, тимчасові файли).  
Вони вказуються у файлі `.gitignore` в корені проєкту. Поточний `.gitignore` вже налаштований для PlatformIO:

```
.pio/
.vscode/
*.pyc
```

---

## 8. Типові помилки

### ❌ `fatal: not a git repository`

**Причина:** Ви не в папці проєкту або проєкт не ініціалізований.

```powershell
# Переконайтесь що ви в правильній папці:
cd c:\Projects_PlatformIO\ESP32__Web_weather_v1_23_04_26

# Або ініціалізуйте git (якщо проєкт новий):
git init
git remote add origin https://github.com/Amator57/esp-32_weather.git
```

---

### ❌ `rejected — non-fast-forward`

**Причина:** На GitHub є коміти, яких немає у вас локально.

```powershell
git pull origin main
git push origin main
```

---

### ❌ `Please tell me who you are`

**Причина:** Git не знає вашого імені та email.

```powershell
git config --global user.name "Ваше Ім'я"
git config --global user.email "your@email.com"
```

---

### ❌ `Authentication failed`

**Причина:** GitHub більше не приймає паролі — потрібен **Personal Access Token**.

1. Перейдіть на GitHub.com → **Settings** → **Developer Settings** → **Personal access tokens** → **Tokens (classic)**
2. Натисніть **Generate new token**
3. Оберіть scope: `repo` (повний доступ до репозиторіїв)
4. Скопіюйте токен (він показується **тільки один раз**)
5. При наступному `git push` введіть **токен замість пароля**

Щоб не вводити токен кожного разу:
```powershell
git config --global credential.helper store
# Після наступного успішного входу токен буде збережено
```

---

### ❌ `CONFLICT (content): Merge conflict in src/main.cpp`

**Причина:** Одні й ті самі рядки змінено і локально, і на GitHub.

1. Відкрийте файл з конфліктом у VS Code
2. VS Code покаже підсвічені блоки конфлікту
3. Оберіть яку версію залишити (або відредагуйте вручну)
4. Видаліть маркери `<<<<<<<`, `=======`, `>>>>>>>`
5. Збережіть файл та виконайте:
```powershell
git add .
git commit -m "Resolved conflict in main.cpp"
```

---

### ❌ Випадково зробив `git add .` і хочу скасувати

```powershell
# Прибрати всі файли зі стейджингу (зміни у файлах залишаться)
git reset HEAD .

# Прибрати один конкретний файл
git reset HEAD src/main.cpp
```

---

## Рекомендований робочий процес для цього проєкту

```
1. Вранці або перед роботою:
   git pull origin main          ← синхронізуватись з GitHub

2. Кодуєте, тестуєте...

3. Зробили щось що працює:
   git add .
   git commit -m "Чіткий опис що зробили"

4. Завершили роботу на сьогодні:
   git push origin main          ← зберегти на GitHub

5. Перед великими змінами (ризикований рефакторинг):
   git checkout -b feature/назва-функції   ← тестуйте в окремій гілці
```

> 💡 **Золоте правило:** Роби маленькі часті коміти з чіткими описами. Краще 5 комітів по одній зміні, ніж один великий коміт з «усе що зробив за тиждень».

---

*Документ складено для проєкту ESP32 Weather Station.*  
*Репозиторій: https://github.com/Amator57/esp-32_weather*
