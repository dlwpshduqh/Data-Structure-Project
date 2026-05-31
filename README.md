# 小卡交換鏈路管理工具

## Proposal Report

### 動機與目標
<!-- 說明為什麼想做這個專題 -->
在 K-pop 蒐集文化中，為了換到心儀的「本命」小卡，粉絲經常需要參與複雜的「多手交換鏈」（例如：持卡 A 換到 B，再用 B 換到 C）。這種過程極其依賴順序邏輯，且隨時可能因交易者毀約而需要調整。

本專案旨在開發一個類 YouTube 播放清單邏輯的管理工具，幫助粉絲紀錄交換路徑、即時插單或撤銷交易，將抽象的交換過程具象化為可管理的資料結構，解決手寫紀錄混亂且難以追蹤交易進度的痛點。

### 競品比較
1. **對抗 Google Sheets 的低效**：在小卡交換這種需要頻繁「微調順序」的場景中，Linked List 的指標切換比起 Excel 的資料搬移具有壓倒性的技術優勢。
2. **補足社群媒體的混亂**：將 IG 上散亂的對話內容「抽象化」為 ADT 封裝，讓原本模糊的交易意向變成具體的、可操作的資料節點。
3. **加強「反悔」的處理成本**：透過 Stack 實現的局部撤銷，比在 Excel 裡按 Ctrl+Z 或翻找 IG 截圖更科學、更精準。

### 預期功能
<!-- 列出預計實作的功能 -->
1. 建立交換鏈路：依序輸入預計交換的小卡資訊，形成一條完整的交易鏈
2. 動態環節調整：可在鏈路中間隨時插入新的交換節點或刪除已取消的節點
3. 交易進度追蹤：標記目前進行到哪一個節點
4. 操作撤銷：若交易失敗可復原至上一個狀態
5. 儲存未排入鏈路的潛在交換邀請，依時間先後順序處理

### 使用技術
<!-- 使用的語言、框架、工具等 -->
使用語言：C++<br>
開發環境：VS code<br>
版本控制：GitHub<br>
核心技術：指標操作、動態記憶體配置、抽象資料型態設計<br>

### Prototype 預計可驗證內容
本專案之 Prototype 旨在透過實作核心邏輯，驗證技術可行性與情境適用性，具體驗證內容如下：<br>
1. 鏈結邏輯正確性 (Linkage Verification)：
    * 驗證使用指標 (Pointers) 建立的 Linked List 是否能精確模擬小卡交換的「順序關係」。
    * 驗證在鏈路中間執行 $O(1)$ 的插入與刪除操作後，指標指向是否依然正確，無資料遺失 (Memory Leak) 情況。
2. 異常處理機制 (Undo Reliability)：
    * 驗證 Stack 結構是否能準確紀錄交易歷史，並在發生毀約時，實現「局部復原」而非像 Excel 般的「全域撤銷」。
    * 確保系統在多次連續 Undo 操作下，仍能維持交易鏈的完整性。
3. 多案併發管理 (Request Fairness)：
    * 驗證 Queue 結構是否能嚴格執行 FIFO (先進先出) 原則處理待交換請求。
    * 測試在多筆交易同時湧入時，系統是否能依時序自動排序，解決 IG/Threads 訊息洗版造成的優先權混亂問題。
4. 效能與直觀度對比 (Efficiency vs. Usability)：
    * 透過實際操作，驗證基於指標的「鏈式管理」在處理頻繁變動的交易時，操作步數是否少於傳統試算表的「手動搬移」。
    * 驗證 CLI 介面是否能清晰呈現「當下持有卡片」與「目標卡片」之間的邏輯路徑。
---

## Prototype Report
```
#include <iostream>
#include <string>
using namespace std;

struct Photocard {
    string memberName;
    string cardVersion;
    Photocard(string name, string version) {
        memberName = name;
        cardVersion = version;
    }
};

// 1. 節點：交換請求 (維持妳剛才的完美結構)
struct TradeRequest {
    string ownerName;
    Photocard haveCard;
    Photocard wantCard;
    TradeRequest* next;

    TradeRequest(string owner, const Photocard& have, const Photocard& want)
        : ownerName(owner), haveCard(have), wantCard(want), next(nullptr) {}

    string describe() const {
        return ownerName + " [持有: " + haveCard.memberName + " " + haveCard.cardVersion
             + " / 尋找: " + wantCard.memberName + " " + wantCard.cardVersion + "]";
    }
};

// 2. 管理者：換卡網路平台
class TradeNetwork {
private:
    TradeRequest* head;

public:
    TradeNetwork() { head = nullptr; }

    ~TradeNetwork() {
        TradeRequest* current = head;
        while (current != nullptr) {
            TradeRequest* nextReq = current->next;
            delete current;
            current = nextReq;
        }
    }

    // 新增使用者的交換請求
    void addRequest(string owner, const Photocard& have, const Photocard& want) {
        TradeRequest* newReq = new TradeRequest(owner, have, want);
        if (head == nullptr) {
            head = newReq;
        } else {
            TradeRequest* current = head;
            while (current->next != nullptr) {
                current = current->next;
            }
            current->next = newReq;
        }
        cout << "\n=> [系統提示] 成功接收 " << owner << " 的請求！\n";
    }

    // 列出目前大廳裡所有的交換請求 (模擬 APP 上的動態牆)
    void printAllRequests() const {
        if (head == nullptr) {
            cout << "\n=> [系統提示] 目前大廳空無一人，快來發布第一個請求吧！\n";
            return;
        }
        
        cout << "\n========== 換卡大廳 ==========\n";
        TradeRequest* current = head;
        int count = 1;
        while (current != nullptr) {
            cout << count << ". " << current->describe() << "\n";
            current = current->next;
            count++;
        }
        cout << "==============================\n";
    }
};

// 3. 模擬 APP 介面的主程式
int main() {
    TradeNetwork app;
    int choice;
    string inputName, inputHaveName, inputHaveVer, inputWantName, inputWantVer;

    cout << "歡迎來到 K-pop 換卡自動媒合平台！\n";

    // 利用 while(true) 建立一個無限迴圈，這就是所有 APP 保持運作的核心邏輯
    while (true) {
        cout << "\n請選擇功能：\n";
        cout << "1. 發布換卡請求 (模擬使用者填寫表單)\n";
        cout << "2. 查看換卡大廳 (模擬 APP 首頁動態牆)\n";
        cout << "3. 離開系統\n";
        cout << "輸入選項 (1-3): ";
        cin >> choice;

        if (choice == 1) {
            // 模擬前端表單輸入
            cout << "請輸入您的暱稱 (勿加空白): ";
            cin >> inputName;
            cout << "請輸入您手上的小卡[成員 版本] (如: Jeno Candy): ";
            cin >> inputHaveName >> inputHaveVer;
            cout << "請輸入您想換到的小卡[成員 版本] (如: Mark ISTJ): ";
            cin >> inputWantName >> inputWantVer;
            
            // 建立新卡並儲存
            Photocard have(inputHaveName, inputHaveVer);
            Photocard want(inputWantName, inputWantVer);
            // 將前端收到的資料，傳給後端的 TradeNetwork
            app.addRequest(inputName, have, want);

        } else if (choice == 2) {
            // 讀取並顯示所有資料
            app.printAllRequests();

        } else if (choice == 3) {
            cout << "\n感謝使用，系統關閉中...\n";
            break; // 打破迴圈，結束程式
            
        } else {
            cout << "\n=> [錯誤] 無效的選項，請重新輸入。\n";
        }
    }

    return 0;
}
```
### 目前進度
<!-- 完成了什麼 -->
* **系統架構初步成型**：成功實作了具備物件導向思維的交換平台基礎。透過自定義的 `Photocard` (資料實體) 與 `TradeRequest` (鏈結節點)，將小卡交換市場的需求抽象化為可程式化處理的資料格式。
* **動態記憶體管理與鏈結**：利用 Singly Linked List (單向鏈結串列) 實作了「換卡大廳」功能。目前系統已能正確執行動態記憶體配置，並透過指標 (`next`) 穩定維護使用者請求的先後順序。
* **使用者互動介面 (CLI)**：開發了基於 `while` 迴圈與邏輯選單的命令列介面，模擬了從前端輸入數據到後端資料結構存儲的完整資訊流。

### 遇到的困難
<!-- 遇到什麼問題、如何解決或打算如何解決 -->
* **數據隱藏與結構簡化之權衡**：在實作過程中，曾面臨是否該過度封裝的掙扎。目前版本選擇使用更直觀的 `struct` 組合，雖然現階段功能較基礎，但確保了指標在複雜鏈路操作時的邏輯正確性。
* **演算法實現的複雜度挑戰**：目前系統僅能完成請求的存儲與列舉，尚未能自動計算出多方交換的路徑。如何將凌亂的鏈結節點，透過條件比對重新組織成一條「成功的交換鏈路」，是目前技術上最大的斷層。

### 下一步計畫
<!-- 接下來要做什麼 -->
* **實作自動化路徑整理演算法**：預計在 `TradeNetwork` 類別中新增 `AutoMatch` 函數。該功能將不再只是線性顯示，而是會遍歷所有節點，根據 `wantCard` 與 `haveCard` 的匹配程度動態重組指標，呈現出交換鏈圖。
* **導入 Stack/Queue 強化功能**：計畫加入「撤回請求」的 Undo 功能（由 Stack 實作）以及「熱門卡片排隊」的公平配對機制（由 Queue 實作），以符合課程對多樣資料結構運用的要求。

---

## Final Report

```
#include <iostream>
#include <string>
#include <queue>
#include <fstream>
#include <vector> // [新增] 用於儲存動態大小的官方資料庫
using namespace std;

// ======= [新增] 官方小卡資料庫結構 =======
struct GroupData {
    string groupName;
    vector<string> members;
    vector<string> versions;
};

// 全域變數：儲存所有團體資訊
vector<GroupData> officialDB;

// 讀取 db_data.txt 建立官方選單
void loadOfficialDB() {
    ifstream inFile("db_data.txt");
    if (!inFile) {
        cout << "=> [警告] 找不到官方資料庫 (db_data.txt)，請確認檔案存在！\n";
        return;
    }

    string type, value;
    while (inFile >> type >> value) {
        if (type == "GROUP") {
            GroupData newGroup;
            newGroup.groupName = value;
            officialDB.push_back(newGroup);
        } else if (type == "MEMBER" && !officialDB.empty()) {
            officialDB.back().members.push_back(value);
        } else if (type == "VERSION" && !officialDB.empty()) {
            officialDB.back().versions.push_back(value);
        }
    }
    inFile.close();
    cout << "=> [系統提示] 成功載入 " << officialDB.size() << " 個官方團體的資料庫！\n";
}
// =========================================

struct Photocard {
    string memberName;
    string cardVersion;
    Photocard(string name, string version) {
        memberName = name;
        cardVersion = version;
    }
};

// 1. 節點：交換請求
struct TradeRequest {
    string ownerName;
    Photocard haveCard;
    Photocard wantCard;
    TradeRequest* next;

    TradeRequest(string owner, const Photocard& have, const Photocard& want)
        : ownerName(owner), haveCard(have), wantCard(want), next(nullptr) {}

    string describe() const {
        return ownerName + " [持有: " + haveCard.memberName + " " + haveCard.cardVersion
             + " / 尋找: " + wantCard.memberName + " " + wantCard.cardVersion + "]";
    }
};

// 2. 管理者：換卡網路平台
class TradeNetwork {
private:
    TradeRequest* head;
    queue<TradeRequest*> popularWaitlist;
    // [新增] 隱藏系統後台熱門卡清單
    vector<string> hotMembers = {"Karina", "Winter", "Mark", "Wonbin"};

    bool isHotCard(const string& memberName) const {
        for (size_t i = 0; i < hotMembers.size(); i++) {
            if (memberName == hotMembers[i]) return true;
        }
        return false;
    }

public:
    TradeNetwork() { 
        head = nullptr; 
        loadFromFile(); 
    }

    ~TradeNetwork() {
        TradeRequest* current = head;
        while (current != nullptr) {
            TradeRequest* nextReq = current->next;
            delete current;
            current = nextReq;
        }
        while (!popularWaitlist.empty()) {
            delete popularWaitlist.front();
            popularWaitlist.pop();
        }
    }

    void loadFromFile() {
        // 1. 讀取大廳資料
        ifstream inFile("trade_data.txt");
        if (inFile) {
            string owner, haveM, haveV, wantM, wantV;
            while (inFile >> owner >> haveM >> haveV >> wantM >> wantV) {
                Photocard have(haveM, haveV);
                Photocard want(wantM, wantV);
                TradeRequest* newReq = new TradeRequest(owner, have, want);
                if (head == nullptr) head = newReq;
                else {
                    TradeRequest* current = head;
                    while (current->next != nullptr) current = current->next;
                    current->next = newReq;
                }
            }
            inFile.close();
        }

        // 2. 讀取排收候補區資料
        ifstream waitFile("waitlist_data.txt");
        if (waitFile) {
            string owner, haveM, haveV, wantM, wantV;
            while (waitFile >> owner >> haveM >> haveV >> wantM >> wantV) {
                Photocard have(haveM, haveV);
                Photocard want(wantM, wantV);
                TradeRequest* newReq = new TradeRequest(owner, have, want);
                popularWaitlist.push(newReq);
            }
            waitFile.close();
        }
    }

    void saveToFile() const {
        // 1. 儲存大廳
        ofstream outFile("trade_data.txt");
        if (outFile) {
            TradeRequest* current = head;
            while (current != nullptr) {
                outFile << current->ownerName << " " 
                        << current->haveCard.memberName << " " << current->haveCard.cardVersion << " "
                        << current->wantCard.memberName << " " << current->wantCard.cardVersion << "\n";
                current = current->next;
            }
            outFile.close();
        }
        // 2. 儲存排收
        ofstream waitFile("waitlist_data.txt");
        if (waitFile) {
            queue<TradeRequest*> tempQ = popularWaitlist;
            while (!tempQ.empty()) {
                TradeRequest* req = tempQ.front();
                waitFile << req->ownerName << " " 
                         << req->haveCard.memberName << " " << req->haveCard.cardVersion << " "
                         << req->wantCard.memberName << " " << req->wantCard.cardVersion << "\n";
                tempQ.pop();
            }
            waitFile.close();
        }
    }

    void addRequest(string owner, const Photocard& have, const Photocard& want) {
        TradeRequest* newReq = new TradeRequest(owner, have, want);
        if (head == nullptr) {
            head = newReq;
        } else {
            TradeRequest* current = head;
            while (current->next != nullptr) {
                current = current->next;
            }
            current->next = newReq;
        }
        cout << "\n=> [系統提示] 成功接收 " << owner << " 的請求！\n";
        saveToFile(); 
    }

    // [升級] 智慧發布 (包含需求分流與供給召回)
    void smartAddRequest(string owner, const Photocard& have, const Photocard& want) {
        // 1. 處理需求 (去排隊還是去大廳)
        if (isHotCard(want.memberName)) {
            cout << "\n=> [系統攔截] 偵測到您尋找的 '" << want.memberName << "' 屬於高人氣熱門卡！\n";
            joinWaitlist(owner, have, want);
        } else {
            addRequest(owner, have, want);
        }

        // 2. 處理供給 (如果拿出來換的是熱卡，觸發召回)
        if (isHotCard(have.memberName)) {
            reactivateHotDemands(have);
        }
    }

    bool removeRequestByIndex(int index) {
        if (head == nullptr || index < 1) {
            return false;
        }
        if (index == 1) {
            TradeRequest* temp = head;
            head = head->next;
            delete temp;
            saveToFile();
            return true;
        }

        TradeRequest* current = head;
        int count = 1;
        while (current->next != nullptr && count < index - 1) {
            current = current->next;
            count++;
        }
        if (current->next == nullptr) {
            return false;
        }

        TradeRequest* temp = current->next;
        current->next = temp->next;
        delete temp;
        saveToFile();
        return true;
    }

    void joinWaitlist(string owner, const Photocard& have, const Photocard& want) {
        TradeRequest* waitReq = new TradeRequest(owner, have, want);
        popularWaitlist.push(waitReq);
        cout << "\n=> [系統提示] " << owner << " 已進入熱門卡排隊序列，目前排隊人數: " 
             << popularWaitlist.size() << " 人。\n";
        saveToFile();
    }

    // [升級] 處理排收第一順位 (結單，並將同卡排收者移轉至大廳)
    // [究極嚴謹版] 處理排收第一順位 (真實配對雙殺 + 精準版本退回)
    void processWaitlist() {
        if (popularWaitlist.empty()) {
            cout << "\n=> [系統提示] 目前沒有人在候補排隊。\n";
            return;
        }

        // 取得排隊第一名的買家
        TradeRequest* buyerReq = popularWaitlist.front();
        Photocard targetCard = buyerReq->wantCard;

        // --- 1. 尋找真實賣家 (掃描 Linked List) ---
        TradeRequest* seller = head;
        TradeRequest* sellerPrev = nullptr;
        bool foundMatch = false;

        while (seller != nullptr) {
            // 精準比對：大廳中有人「持有」這張特定版本的小卡
            if (seller->haveCard.memberName == targetCard.memberName && 
                seller->haveCard.cardVersion == targetCard.cardVersion) {
                foundMatch = true;
                break;
            }
            sellerPrev = seller;
            seller = seller->next;
        }

        // 防呆：如果大廳根本沒人有這張卡，拒絕結單！
        if (!foundMatch) {
            cout << "\n=> [系統警告] 大廳中目前沒有人擁有 [" << targetCard.memberName << " " << targetCard.cardVersion << "] 的現貨！\n";
            cout << "=> 無法強制結單，請等待賣家釋出。\n";
            return;
        }

        // --- 2. 拔除賣家 (從 Linked List 刪除) ---
        if (sellerPrev == nullptr) {
            head = seller->next;
        } else {
            sellerPrev->next = seller->next;
        }
        string sellerName = seller->ownerName;
        delete seller; // 釋放賣家記憶體

        // --- 3. 拔除買家 (從 Queue 刪除) ---
        popularWaitlist.pop();

        cout << "\n========== 🎉 真實結單出貨完成 🎉 ==========\n";
        cout << "=> 恭喜排位第一的買家 [" << buyerReq->ownerName << "]\n";
        cout << "=> 成功與大廳賣家 [" << sellerName << "] 配對交易！\n";
        cout << "=> 已將雙方的請求從系統中徹底刪除結單。\n";
        cout << "==========================================\n";
        delete buyerReq; // 釋放買家記憶體

        // --- 4. 剩餘排收者退回大廳 (精準比對：只退回排同一張特定卡的人) ---
        queue<TradeRequest*> remainingQueue;
        int movedCount = 0;

        while (!popularWaitlist.empty()) {
            TradeRequest* req = popularWaitlist.front();
            popularWaitlist.pop();

            // [嚴謹比對] 必須「成員」與「版本」完全一樣，才代表他們在搶同一張現貨
            if (req->wantCard.memberName == targetCard.memberName && 
                req->wantCard.cardVersion == targetCard.cardVersion) {
                req->next = nullptr; 
                if (head == nullptr) {
                    head = req;
                } else {
                    TradeRequest* current = head;
                    while (current->next != nullptr) current = current->next;
                    current->next = req;
                }
                movedCount++;
            } else {
                // 想要其他版本或其他成員的人，不受影響，繼續留在 Queue 裡排隊！
                remainingQueue.push(req);
            }
        }

        popularWaitlist = remainingQueue;

        if (movedCount > 0) {
            cout << "=> [系統提示] [" << targetCard.memberName << " " << targetCard.cardVersion << "] 的熱卡現貨已盡！\n";
            cout << "=> 其餘 " << movedCount << " 位排收同張卡的玩家已轉移至大廳繼續尋找機會 (解除 HOT 標籤)。\n";
        }
        saveToFile(); 
    }

    // [新增] 供給觸發：當熱卡釋出時，召回大廳中的需求者
    void reactivateHotDemands(const Photocard& suppliedCard) {
        TradeRequest* current = head;
        TradeRequest* prev = nullptr;
        int reactivatedCount = 0;

        while (current != nullptr) {
            if (current->wantCard.memberName == suppliedCard.memberName && 
                current->wantCard.cardVersion == suppliedCard.cardVersion) {
                TradeRequest* toMove = current;
                if (prev == nullptr) head = current->next;
                else prev->next = current->next;
                current = current->next; 
                toMove->next = nullptr; 
                popularWaitlist.push(toMove);
                reactivatedCount++;
            } else {
                prev = current;
                current = current->next;
            }
        }

        if (reactivatedCount > 0) {
            cout << "\n=> 🌟 [市場震盪] 偵測到熱門卡 [" << suppliedCard.memberName << " " << suppliedCard.cardVersion << "] 釋出！\n";
            cout << "=> 已將大廳中 " << reactivatedCount << " 位相關需求者重新召回排收佇列 (恢復 HOT 標籤)！\n";
            saveToFile(); 
        }
    }

    void autoMatch() {
        if (head == nullptr || head->next == nullptr) {
            cout << "\n=> [系統提示] 大廳人數不足，無法進行媒合。\n";
            return;
        }

        cout << "\n========== 系統自動媒合結果 ==========\n";
        bool foundMatch = false;

        cout << "[尋找雙向換卡 (2-way)]\n";
        TradeRequest* p1 = head;
        while (p1 != nullptr) {
            TradeRequest* p2 = p1->next;
            while (p2 != nullptr) {
                if (p1->wantCard.memberName == p2->haveCard.memberName && p1->wantCard.cardVersion == p2->haveCard.cardVersion &&
                    p2->wantCard.memberName == p1->haveCard.memberName && p2->wantCard.cardVersion == p1->haveCard.cardVersion) {
                    cout << "  完美配對: " << p1->ownerName << " <---> " << p2->ownerName << "\n";
                    foundMatch = true;
                }
                p2 = p2->next;
            }
            p1 = p1->next;
        }

        cout << "\n[尋找三向換卡鏈 (3-way)]\n";
        p1 = head;
        while (p1 != nullptr) {
            TradeRequest* p2 = head;
            while (p2 != nullptr) {
                if (p1 == p2) { p2 = p2->next; continue; }
                if (p1->wantCard.memberName == p2->haveCard.memberName && p1->wantCard.cardVersion == p2->haveCard.cardVersion) {
                    TradeRequest* p3 = head;
                    while (p3 != nullptr) {
                        if (p3 == p1 || p3 == p2) { p3 = p3->next; continue; }
                        if (p2->wantCard.memberName == p3->haveCard.memberName && p2->wantCard.cardVersion == p3->haveCard.cardVersion &&
                            p3->wantCard.memberName == p1->haveCard.memberName && p3->wantCard.cardVersion == p1->haveCard.cardVersion) {
                            if (p1 < p2 && p1 < p3) {
                                cout << "  🔄 三方閉環: " << p1->ownerName << " 換給 -> " << p2->ownerName 
                                     << " 換給 -> " << p3->ownerName << " 換回 -> " << p1->ownerName << "\n";
                                foundMatch = true;
                            }
                        }
                        p3 = p3->next;
                    }
                }
                p2 = p2->next;
            }
            p1 = p1->next;
        }

        if (!foundMatch) cout << "=> 目前大廳內尚未找到 2-way 或 3-way 的交換鏈。\n";
        cout << "============================================\n";
    }

    // [升級版] 列出目前大廳與排收的所有請求 (視覺整合)
    void printAllRequests() const {
        cout << "\n========== 換卡大廳 ==========" << "\n";
        int count = 1;

        // 1. 先印出一般大廳的現貨 (來自 Linked List)
        TradeRequest* current = head;
        while (current != nullptr) {
            cout << count << ". " << current->describe() << "\n";
            current = current->next;
            count++;
        }

        // 2. 接著印出排收中的熱門卡 (來自 Queue)，讓它們也能被看見！
        if (!popularWaitlist.empty()) {
            queue<TradeRequest*> tempQ = popularWaitlist;
            while (!tempQ.empty()) {
                TradeRequest* req = tempQ.front();
                cout << count << ". " << req->describe() << " [🔥 HOT 排收中]\n";
                tempQ.pop();
                count++;
            }
        }

        if (count == 1) {
            cout << "=> [系統提示] 目前大廳空無一人，快來發布第一個請求吧！\n";
        }
        cout << "==============================\n";
    }

    // [升級版功能 D] 專屬動態牆與市場數據分析
    void checkMyDashboard(string userName) const {
        cout << "\n========== [" << userName << "] 的專屬動態牆 ==========" << "\n";
        
        // --- 區塊 1：大廳現貨區狀態 (Linked List 遍歷) ---
        cout << "\n【大廳配對與熱度分析】\n";
        bool hasRequest = false;
        TradeRequest* myReq = head;

        while (myReq != nullptr) {
            if (myReq->ownerName == userName) {
                hasRequest = true;
                cout << "=> 您的請求 [持有: " << myReq->haveCard.memberName << " " << myReq->haveCard.cardVersion 
                     << " / 尋找: " << myReq->wantCard.memberName << " " << myReq->wantCard.cardVersion << "]\n";

                TradeRequest* other = head;
                bool foundAny = false;
                int demandCount = 0;     // 統計多少人想要我的卡
                int competitorCount = 0; // 統計多少人跟我在搶同一張卡

                // 掃描大廳其他人
                while (other != nullptr) {
                    if (other->ownerName == userName) { other = other->next; continue; }

                    bool iWantHis = (myReq->wantCard.memberName == other->haveCard.memberName && myReq->wantCard.cardVersion == other->haveCard.cardVersion);
                    bool heWantsMine = (other->wantCard.memberName == myReq->haveCard.memberName && other->wantCard.cardVersion == myReq->haveCard.cardVersion);

                    // 1. 配對通知
                    if (iWantHis && heWantsMine) {
                        cout << " [第一順位] " << other->ownerName << " 剛好可以跟您互換！\n"; foundAny = true;
                    } else if (iWantHis) {
                        cout << " [第二順位] " << other->ownerName << " 手上有您想要的卡！(但他想換 " << other->wantCard.memberName << ")\n"; foundAny = true;
                    } else if (heWantsMine) {
                        cout << " [第三順位] " << other->ownerName << " 正在尋找您手上的卡！\n"; foundAny = true;
                    }

                    // 2. 數據統計
                    if (heWantsMine) demandCount++;
                    if (other->wantCard.memberName == myReq->wantCard.memberName && other->wantCard.cardVersion == myReq->wantCard.cardVersion) competitorCount++;

                    other = other->next;
                }
                
                // 印出市場數據
                cout << " [市場數據] 大廳目前有 " << demandCount << " 人想要您的卡！";
                if (competitorCount > 0) cout << " (另有 " << competitorCount << " 人正在跟您競爭想換的卡)\n";
                else cout << "\n";

                if (!foundAny) cout << "  => 目前大廳還沒有適合的換卡機會。\n";
                cout << "------------------------------------------\n";
            }
            myReq = myReq->next;
        }
        if (!hasRequest) cout << "=> 您目前在大廳沒有發布任何請求。\n";

        // --- 區塊 2：排收區狀態 (Queue 查詢) ---
        cout << "\n【🔥 熱門卡專屬排收進度】\n";
        bool foundInQueue = false;
        
        // 技巧：因為 Queue 無法直接走訪，我們將它複製到 vector 進行安全分析
        queue<TradeRequest*> tempQ = popularWaitlist;
        vector<TradeRequest*> qList;
        while (!tempQ.empty()) {
            qList.push_back(tempQ.front());
            tempQ.pop();
        }

        for (size_t i = 0; i < qList.size(); i++) {
            if (qList[i]->ownerName == userName) {
                foundInQueue = true;
                int pos = 1; // 預設自己是第 1 順位
                
                // 往前算看有幾個人跟我「排收同一張卡」
                for (size_t j = 0; j < i; j++) {
                    if (qList[j]->wantCard.memberName == qList[i]->wantCard.memberName &&
                        qList[j]->wantCard.cardVersion == qList[i]->wantCard.cardVersion) {
                        pos++; // 前面多一個人，我的順位就往後延 1 號
                    }
                }
                cout << "=> ⏳ 正在排收 (熱門卡): " << qList[i]->wantCard.memberName << " " << qList[i]->wantCard.cardVersion 
                     << " | 您目前的排收序號是: 第 " << pos << " 順位\n";
            }
        }
        if (!foundInQueue) cout << "=> 您目前沒有參與任何熱門卡的排收。\n";
        cout << "=================================================\n";
    }
};

// ======= [新增] 連動式防呆選單：一次性選擇團體、成員與版本 =======
Photocard createCardFromDB(string promptMsg) {
    cout << "\n[" << promptMsg << "]\n";
    if (officialDB.empty()) {
        cout << "=> [系統警告] 資料庫為空，請手動輸入成員與版本。\n";
        string m, v; cout << "成員: "; cin >> m; cout << "版本: "; cin >> v;
        return Photocard(m, v);
    }

    // 1. 選擇團體
    cout << "--- 選擇團體 ---\n";
    for (size_t i = 0; i < officialDB.size(); i++) {
        cout << i + 1 << ". " << officialDB[i].groupName << "\n";
    }
    int gChoice;
    while (true) {
        cout << "請輸入編號 (1-" << officialDB.size() << "): ";
        cin >> gChoice;
        if (cin.fail() || gChoice < 1 || gChoice > officialDB.size()) {
            cin.clear(); cin.ignore(10000, '\n'); cout << "=> [錯誤] 無效的編號。\n";
        } else break;
    }
    GroupData& selectedGroup = officialDB[gChoice - 1];

    // 2. 選擇該團體的成員
    cout << "\n--- 選擇 [" << selectedGroup.groupName << "] 的成員 ---\n";
    for (size_t i = 0; i < selectedGroup.members.size(); i++) {
        cout << i + 1 << ". " << selectedGroup.members[i] << "\n";
    }
    int mChoice;
    while (true) {
        cout << "請輸入編號 (1-" << selectedGroup.members.size() << "): ";
        cin >> mChoice;
        if (cin.fail() || mChoice < 1 || mChoice > selectedGroup.members.size()) {
            cin.clear(); cin.ignore(10000, '\n'); cout << "=> [錯誤] 無效的編號。\n";
        } else break;
    }

    // 3. 選擇該團體的版本
    cout << "\n--- 選擇 [" << selectedGroup.groupName << "] 的卡片版本 ---\n";
    for (size_t i = 0; i < selectedGroup.versions.size(); i++) {
        cout << i + 1 << ". " << selectedGroup.versions[i] << "\n";
    }
    int vChoice;
    while (true) {
        cout << "請輸入編號 (1-" << selectedGroup.versions.size() << "): ";
        cin >> vChoice;
        if (cin.fail() || vChoice < 1 || vChoice > selectedGroup.versions.size()) {
            cin.clear(); cin.ignore(10000, '\n'); cout << "=> [錯誤] 無效的編號。\n";
        } else break;
    }

    return Photocard(selectedGroup.members[mChoice - 1], selectedGroup.versions[vChoice - 1]);
}
// ==============================================================

// 3. 模擬 APP 介面的主程式
int main() {
    loadOfficialDB(); // 程式啟動第一件事：載入資料庫

    TradeNetwork app;
    int choice;
    string inputName;

    cout << "歡迎來到 K-pop 換卡自動媒合平台！\n";

    while (true) {
        cout << "\n請選擇功能：\n";
        cout << "1. 發布換卡請求 (系統自動判定現貨/排收)\n";
        cout << "2. 查看換卡大廳 (公開動態牆)\n";
        cout << "3. 刪除指定換卡請求\n";
        cout << "4. [管理員] 處理排收第一順位 (Dequeue)\n";
        cout << "5. 執行自動媒合尋找交換鏈 (2-way/3-way)\n";
        cout << "6. 查看專屬動態牆 (大廳配對機會 & 排收序號查詢)\n";
        cout << "7. 離開系統\n";
        cout << "輸入選項 (1-7): ";
        cin >> choice;

        if (cin.fail()) {
            cin.clear(); cin.ignore(10000, '\n');
            cout << "\n=> [錯誤] 請輸入數字選項！\n";
            continue;
        }

        if (choice == 1) {
            cout << "請輸入您的暱稱 (勿加空白): ";
            cin >> inputName;
            
            Photocard have = createCardFromDB("設定您手上的小卡");
            Photocard want = createCardFromDB("設定您想換到的小卡");
            app.smartAddRequest(inputName, have, want);

        } else if (choice == 2) {
            app.printAllRequests();

        } else if (choice == 3) {
            app.printAllRequests();
            cout << "請輸入要刪除的請求編號: ";
            int deleteIndex;
            cin >> deleteIndex;
            if (cin.fail()) {
                cin.clear();
                cin.ignore(10000, '\n');
                cout << "\n=> [錯誤] 請輸入有效的數字。\n";
            } else if (app.removeRequestByIndex(deleteIndex)) {
                cout << "\n=> [系統提示] 已成功刪除第 " << deleteIndex << " 筆請求。\n";
            } else {
                cout << "\n=> [系統提示] 無效編號或找不到該請求。\n";
            }

        } else if (choice == 4) {
            app.processWaitlist();

        } else if (choice == 5) {
            app.autoMatch();

        } else if (choice == 6) {
            cout << "請輸入您的暱稱，以查看動態牆與排收狀態: ";
            cin >> inputName;
            app.checkMyDashboard(inputName);

        } else if (choice == 7) {
            cout << "\n感謝使用，系統關閉中...\n";
            break;
        } else {
            cout << "\n=> [錯誤] 無效的選項，請重新輸入。\n";
        }
    }
    return 0;
}
```

### 專案說明<br>
本專案為「K-pop 小卡自動媒合與排收平台」，旨在解決真實粉絲社群中，換卡資訊洗版、成員名字拼寫錯誤，以及多方交易難以人工比對等痛點。系統採用 C++ 開發，將底層資料結構與真實市場供需邏輯深度結合：<br>
<br>
**動態儲存與效能優化：** 以 Linked List 實作換卡大廳，支援 O(1) 高效指定刪除（下架請求），免除傳統陣列搬移的效能成本。<br>
**排收公平與供需震盪：** 以 Queue 實作熱門卡候補區，保障先來後到 (FIFO)。若市場釋出熱門卡供給，系統會自動將大廳需求者召回佇列；管理員結單出貨時，則精準配對買賣雙方，並將無卡可換的剩餘排收者退回大廳。<br>
**高階媒合與防呆持久化：** 內建 O(N^3) 圖論有向環偵測演算法，自動尋找 2-way 與 3-way 完美交換鏈。系統啟動時讀取官方資料庫限制輸入防呆，並在每次操作後將狀態雙檔同步至本地端 (`trade_data.txt` 與 `waitlist_data.txt`)。<br>
<br>
### 使用方式<br>
**環境準備與編譯執行**<br>
 1. 執行前請確保程式同目錄下具備 `db_data.txt`（官方團體/成員/版本資料庫）。<br>
 2. 使用 C++ 編譯器進行編譯（例如指令：`g++ main.cpp -o trade_app`）。<br>
 3. 執行產生的執行檔（例如指令：`./trade_app` 或點擊 `.exe` 檔）。<br>
<br>
**系統操作指南**<br>
程式啟動後將自動載入歷史存檔，請依終端機提示輸入對應數字 (1-7) 選擇功能：<br>
<br>
**1. 發布換卡請求：** 透過防呆數字選單設定小卡。系統會自動判定為一般現貨（進入大廳）或熱門卡（攔截至 Queue）。釋出熱卡將觸發市場震盪召回機制。<br>
**2. 查看換卡大廳：** 瀏覽目前所有的現貨與候補清單，熱門卡需求會自動標註 `[🔥 HOT 排收中]` 標籤。<br>
**3. 刪除指定請求：** 依畫面顯示的大廳編號，透過 O(1) 指標重接安全下架指定的換卡請求。<br>
**4. [管理員] 結單出貨 (Dequeue)：** 系統自動精準配對 Queue 第一順位買家與大廳持卡賣家，完成交易並清除雙方紀錄；其餘排收同張特定熱卡的使用者將被退回大廳。<br>
**5. 執行自動媒合：** 啟動後台演算法，掃描並印出目前大廳內所有 2-way (雙向) 與 3-way (三方閉環) 的配對機會。<br>
**6. 查看專屬動態牆：** 輸入使用者暱稱，查詢個人請求的市場競爭者數量、配對機會，以及當下精準的「絕對排收順位」。<br>
**7. 離開系統：** 安全關閉程式，所有資料皆已即時持久化存檔。<br>

