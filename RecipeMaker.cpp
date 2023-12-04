#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include "openai.hpp"
#include <nlohmann/json.hpp>
#include <curl/curl.h>
//#include "opencv2/opencv.hpp"
using json = nlohmann::json;

using namespace std;

class CookingHelper {
private:
    string Ing;
    string Menu;
    string Allergy;
    string Diets;
    string Exclude_Menu;
    string Taste;
    vector<std::string> RecipeArray;

public:
    void setIng(const string& UserIngs) {
        Ing = UserIngs;
    }

    void setMenu(const string& UserMenu) {
        Menu = UserMenu;
    }

    void setAllergy(const string& UserAllergy) {
        Allergy = UserAllergy;
    }

    void setExclude(const string& UserEcMenu){
        Exclude_Menu = UserEcMenu;
    }

    void setTaste(const string& UserTaste) {
        Taste = UserTaste;
    }    

    string getIng(){
        return Ing;
    }

    string getMenu(){
        return Menu;
    }

    vector<std::string> getRecipe(){
        return RecipeArray;
    }

    string getAllergy(){
        return Allergy;
    }

    string getTaste(){
        return Taste;
    }

    void print(const string& str){
        cout << str << endl << endl;
    }

    void print(const vector<string>& strs){

        for(const auto& c : strs){
            std::cout << c << std::endl;
        }
        std::cout << std::endl;
        
    }

    string recomend_menu(int N = 5) {
        json requestData;
        string num = to_string(N);
        requestData["model"] = "gpt-3.5-turbo";
        requestData["messages"][0]["role"] = "user";
        //cout << "제외된 메뉴들 : " << Exclude_Menu << endl;
        requestData["messages"][0]["content"] = Exclude_Menu + "를 제외하고!!!!" + Ing + "(을)를 사용해서 만들 수 있는 " + Taste + " 음식은 뭐가 있어? " + num + 
                                                "개만 추천해줘!!!모든 재료를 사용할 필요는 없고, 다른 재료를 사용해도 괜찮아. 그래도 " + Allergy + 
                                                "는 사용하지마!! 설명 하지말고 음식만 나열해줘.";
        requestData["max_tokens"] = 3000;
        requestData["temperature"] = 0.5;
        //cout << requestData["messages"][0]["content"] << endl;

        auto response = openai::chat().create(requestData);

        string RecommendMenu = response["choices"][0]["message"]["content"].get<std::string>();
        return RecommendMenu;
        //cout << Recipe << endl;
    }

    void GenerageRecipe(const string& menu) {
        
        json requestData;
        requestData["model"] = "gpt-4-1106-preview";
        requestData["messages"][0]["role"] = "user";
        requestData["messages"][0]["content"] =  "메뉴 : " + menu + "\n레시피 알려줘. 1. 재료 2. 재료 손질법 3. 가열법 4. 조리법 , 이렇게 1. 2. 3. 4. 로 나누어 레시피 정렬해서 알려줘. 번역투 쓰지 말아줘.";
        requestData["max_tokens"] = 3000;
        requestData["temperature"] = 0.5;

        auto response = openai::chat().create(requestData);
        string generatedText = response["choices"][0]["message"]["content"].get<std::string>();

        std::stringstream ss(generatedText);
        std::string line;
        bool fisrt = true;
        bool once = true;
        while (std::getline(ss, line, '\n')) {
            if(fisrt) {
                RecipeArray.push_back(line);
                fisrt = false;
            }
            if((line[0] >= '1' && line[0] <= '4')|| line[0] == '-'){
                RecipeArray.push_back(line);
                once = true;
            }
            if(line.empty()&&once){
                RecipeArray.push_back(line);
                once = false;
            }
                       
        }
        RecipeArray.push_back("\n");
        //print(RecipeArray);
    }
    

};

class DietHelper : public CookingHelper{
private :
    int days;
    string BuyList;
public :  
    void setDays(const string& setdays){
        days = stoi(setdays);
    }

    int getDays(){
        return days;
    }

    string getBuyList(){
        return BuyList;
    }

    void GenerateDietPlan() {
        string recommendedMenu;
        recommendedMenu = recomend_menu(days);
        setMenu(recommendedMenu);
        //cout << "메뉴들\n" << recommendedMenu << endl;

        std::stringstream ss(recommendedMenu);
        std::string line;
        while (std::getline(ss, line, '\n')) {
            GenerageRecipe(line);
        }
    }
    
    void GetList(){
        string Menus = getMenu();

        json requestData;       
        requestData["prompt"] = Menus +  "를 만들기 위한 통합된 장바구니 리스트를 만들어줘. " + getIng() + "는 이미 집에 있으므로 그 외에 필요한 재료를 나열해줘.";
        requestData["model"] = "text-davinci-003";
        requestData["max_tokens"] = 3000;
        requestData["temperature"] = 0.4;

        auto response = openai::completion().create(requestData);
        BuyList = response["choices"][0]["text"].get<string>();
    }

    void saveRecipeToFile(int is_List){
        std::ofstream file("식단표.txt");
        if(is_List != 2){
            file << "<<<<<장바구니>>>>>" << std::endl; 

            size_t pos;
            while ((pos = BuyList.find("\n\n")) != std::string::npos) {
                std::cout << "Found at position: " << pos << std::endl;
                BuyList.erase(pos, 2); // 1개의 문자를 삭제합니다.
            }

            if (file.is_open()) {
                file << BuyList << std::endl; 
                std::cout << "장바구니 저장 완료" << std::endl;
            } else {
                std::cout << "파일을 열 수 없습니다." << std::endl;
            }
        }

        file << std::endl << std::endl << "<<<<<레시피>>>>>" << std::endl; 

        if (file.is_open()) {
            for (const auto& line : getRecipe()) {
                file << line << std::endl; // 각 섹션을 파일에 씁니다.
            }
            file.close();
            std::cout << "레시피가 식단표.txt 파일로 저장되었습니다." << std::endl;
        } 
        else {
            std::cout << "파일을 열 수 없습니다." << std::endl;
        }
    }

};

class HealthHelper : public CookingHelper{
private :
    string Nut;
    string WOInfo;
public : 
    string get_Nut(){
        return Nut;
    }

    string get_WOInfo(){
        return WOInfo;
    }

    void FindNut(){
        string menu = getMenu();
        json requestData;       
        requestData["prompt"] = menu +  "의 영양정보(칼로리, 탄수화물, 단백질, 지방)를 알려줘.";
        requestData["model"] = "text-davinci-003";
        requestData["max_tokens"] = 3000;
        requestData["temperature"] = 0.4;
    
        auto response = openai::completion().create(requestData);
        Nut = response["choices"][0]["text"].get<string>();
    }

    void FindWokrOut(){
        string menu = getMenu();
        json requestData;       
        requestData["prompt"] = menu +  "를 소모할 수 있는 운동 및 운동별 시간을 알려줘";
        requestData["model"] = "text-davinci-003";
        requestData["max_tokens"] = 3000;
        requestData["temperature"] = 0.4;
    
        auto response = openai::completion().create(requestData);
        WOInfo = response["choices"][0]["text"].get<string>();
    }



};

int main() {    
        string Progarm, Retry;
        CookingHelper Info;
        string UserAllergy, UserTaste, UserIngs;
        std::cout << "냉장고 속 재료 정보 및 사용자 정보를 수집합니다."<< std::endl << std::endl;

        std::cout << "싫어하는 식재료를 알려주세요\n\n" << "음식 : ";
        getline(cin, UserAllergy);
        Info.setAllergy(UserAllergy);

        std::cout << "냉장고에 존재하는 재료를 5가지 이상 알려주세요.\n\n" << "재료 : ";
        getline(cin, UserIngs);
        Info.setIng(UserIngs);
        std::cout<< std::endl;

        cout << "식사 취향을 알려주세요.\n" << "예시 : 육식 위주의/채식 위주의/고단백 위주의/저탄수화물 위주의/지중해식 위주의/한식 위주의\n" <<"취향 : ";
        getline(cin, UserTaste);
        Info.setTaste(UserTaste);
        std::cout<< std::endl;

        std::cout << "냉장고 속 재료 정보 및 사용자 정보를 이용해 만든 프로그램들이 있습니다.\n어떤 프로그램을 사용하길 원하시나요?\n"
                    << "1.식단 플래너 2.운동 플래너 (숫자로 입력해 주세요)" << std::endl;
        getline(cin, Progarm);
        std::cout << std::endl;

        int P_ctl = 1;
        while(P_ctl == 1){
            if(Progarm == "1"){
                // 식단 플래너 시작
                P_ctl = 0;
                DietHelper d_helper;
                string s_day, response;

                d_helper.setAllergy(Info.getAllergy());
                d_helper.setIng(Info.getIng());
                d_helper.setTaste(Info.getTaste());

                std::cout << "식단은 며칠을 원하나요? (숫자 하나만 입력해주세요. 최대 7일 입니다.)" << std::endl;
                getline(cin, s_day);
                std::cout << std::endl;
                d_helper.setDays(s_day);

                std::cout << "식단을 생성중입니다." << std::endl;
                d_helper.GenerateDietPlan();
                std::cout << "식단 생성 완료" << std::endl << std::endl;
                d_helper.print(d_helper.getRecipe());
                
                std::cout << "장바구니 리스트를 보시겠습니까? (Y/N)" << std::endl;
                getline(cin, response);
                int controLL = 1;
                while(controLL == 1){
                    if(response == "Y")
                    {
                        d_helper.GetList();
                        controLL = 0;
                    }
                    else if(response == "N"){
                        controLL = 2;
                        break;
                    }
                    else{
                        std::cout << "(Y/N) 중에 골라 주세요" << std::endl;
                        getline(cin, response);
                    }
                }
                d_helper.print(d_helper.getBuyList());

                std::cout << "레시피와 장바구니 리스트를 파일로 저장해 드릴까요? (Y/N)" << std::endl << std::endl;
                getline(cin, response);
                int control3 = 1;
                while(control3 == 1){
                    if(response == "Y")
                    {
                        d_helper.saveRecipeToFile(controLL);
                        control3 = 0;
                    }
                    else if(response == "N"){
                        return 0;
                    }
                    else{
                        std::cout << "(Y/N) 중에 골라 주세요" << std::endl;
                        getline(cin, response);
                    }
                }


                std::cout << "\n다른 프로그램을 사용하길 원하나요? (Y/N)" << endl;
                getline(cin, Retry);
                int R_ctl = 1;
                while(R_ctl == 1){
                    if(Retry == "Y"){
                        P_ctl = 2;
                        R_ctl = 0;
                    }
                    else if(Retry == "N"){
                        std::cout << "프로그램을 종료합니다" << endl;
                        return 0;
                    }
                    else{
                        std::cout << "(Y/N) 중에 골라 주세요" << std::endl;
                        getline(cin, Retry);
                    }
                }

            }
            else if(Progarm == "2"){
                // 운동 플래너 시작
                P_ctl = 0;
                HealthHelper h_helper;
                string ntr, Recommend, menu, response;

                h_helper.setAllergy(Info.getAllergy());
                h_helper.setIng(Info.getIng());
                h_helper.setTaste(Info.getTaste());

                std::cout << "냉장고 속 재료 정보 및 사용자 정보를 통해 도출된 추천 메뉴 입니다." << std::endl;
                Recommend = h_helper.recomend_menu();
                h_helper.print(Recommend);
                std::cout << std::endl;

                std::cout << "영양 성분을 알고 싶은 메뉴를 입력해 주세요" << std::endl;
                getline(cin, menu);
                h_helper.setMenu(menu);
                std::cout << std::endl;

                std::cout << menu << "의 영양성분은 다음과 같습니다." << std::endl;
                h_helper.FindNut();
                h_helper.print(h_helper.get_Nut());

                std::cout << menu << "를 소모할 수 있는 운동방법을 알려드릴까요? (Y/N)" << std::endl;
                getline(cin, response);

                int control = 1;
                while(control == 1){
                    if(response == "Y"){
                        h_helper.FindWokrOut();
                        std::cout << "운동법은 다음과 같습니다." << std::endl;
                        h_helper.print(h_helper.get_WOInfo());
                        control = 0;
                    }
                    else if(response == "N"){
                        control = 0;
                    }
                    else{
                        std::cout << "(Y/N) 중에 골라 주세요" << std::endl;
                        getline(cin, response);
                    }
                }

                std::cout << "\n다른 프로그램을 사용하길 원하나요? (Y/N)" << endl;
                getline(cin, Retry);
                int R_ctl = 1;
                while(R_ctl == 1){
                    if(Retry == "Y"){
                        P_ctl = 2;
                        R_ctl = 0;
                    }
                    else if(Retry == "N"){
                        std::cout << "프로그램을 종료합니다" << endl;
                        return 0;
                    }
                    else{
                        std::cout << "(Y/N) 중에 골라 주세요" << std::endl;
                        getline(cin, Retry);
                    }
                }
            }
            else{
                std::cout << "숫자만 입력해 주세요" << std::endl;
                getline(cin, Progarm);
            }

            if(P_ctl == 2){
                P_ctl = 1;
                std::cout << "냉장고 속 재료 정보를 이용해 만든 프로그램들이 있습니다.\n어떤 프로그램을 사용하길 원하시나요?\n"
                            << "1.식단 플래너 2.운동 플래너 (숫자로 입력해 주세요)" << endl;
                getline(cin, Progarm);
                std::cout << std::endl;
            }
        }

        
    return 0;
}
