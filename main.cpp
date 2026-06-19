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