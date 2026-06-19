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

### 專案說明
<!-- 完整描述你的專案做了什麼 -->

### 使用方式
<!-- 如何編譯、執行、使用你的程式 -->

