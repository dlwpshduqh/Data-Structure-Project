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