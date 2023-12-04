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
    string BuyList;
    vector<std::string> MenuArray;
    vector<std::string> RecipeArray;

    int days;

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

    void setDays(const string& setdays){
        days = stoi(setdays);
    }

    void setTaste(const string& UserTaste) {
        Taste = UserTaste;
    }    

    vector<std::string> getRecipe(){
        return RecipeArray;
    }

    int getDays(){
        return days;
    }

    string getBuyList(){
        return BuyList;
    }

    string recomend_menu(int N = 5) {
        json requestData;
        string num = to_string(N);
        requestData["model"] = "gpt-3.5-turbo";
        requestData["messages"][0]["role"] = "user";
        //cout << "제외된 메뉴들 : " << Exclude_Menu << endl;
        requestData["messages"][0]["content"] = Exclude_Menu + "를 제외하고!!!!" + Ing + "(을)를 사용해서 만들 수 있는" + Taste + "음식은 뭐가 있어?" + num + 
                                                "개만 추천해줘!!!모든 재료를 사용할 필요는 없고, 다른 재료를 사용해도 괜찮아. 그래도" + Allergy + 
                                                "는 사용하지마!! 설명 하지말고 음식만 나열해줘.";
        requestData["max_tokens"] = 3000;
        requestData["temperature"] = 0.5;

        auto response = openai::chat().create(requestData);

        string RecommendMenu = response["choices"][0]["message"]["content"].get<std::string>();
        return RecommendMenu;
        //cout << Recipe << endl;
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

    void GenerageRecipe() {
        
        json requestData;
        requestData["model"] = "gpt-4-1106-preview";
        requestData["messages"][0]["role"] = "user";
        requestData["messages"][0]["content"] =  "메뉴 : " + Menu + "\n레시피 알려줘. 1. 재료 2. 재료 손질법 3. 가열법 4. 조리법 , 이렇게 1. 2. 3. 4. 로 나누어 레시피 정렬해서 알려줘. 번역투 쓰지 말아줘.";
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
    }

    void GenerageDiet() {

        for(int i = 0; i < days - 1 ; i++){
            string num = to_string(i);

            
            json requestData;
            requestData["model"] = "gpt-3.5-turbo";
            requestData["messages"][0]["role"] = "user";
            requestData["messages"][0]["content"] =  "메뉴 : " + MenuArray[i] + "\n레시피 알려줘. 1. 재료 2. 재료 손질법 3. 가열법 4. 조리법 , 이렇게 1. 2. 3. 4. 로 나누어 레시피 정렬해서 알려줘. 번역투 쓰지 말아줘.";
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
                if((line[0] >= '1' && line[0] <= '4')|| line[0] == '-' || line[0] == '\n'){
                    RecipeArray.push_back(line);
                    once = true;
                }
                if(line.empty()&&once){
                    RecipeArray.push_back(line);
                    once = false;
                }
            }
            RecipeArray.push_back("\n");
        }
    }

    void DietRecommend() {
        string recommendedMenu;
        int N = days-1;
        recommendedMenu += recomend_menu(N);

        std::stringstream ss(recommendedMenu);
        std::string line;
        while (std::getline(ss, line, '\n')) {
            MenuArray.push_back(line);
        }
        GenerageDiet();
    }

    void GetList(){
        string Menus = Menu;
        for(int i=0; i<days-1; i++){
            Menus += MenuArray[i];
        }

        json requestData;
        //std::cout << "Getlist에서 들어가는 Menu : " << Menus  << endl;
        
        requestData["prompt"] = Menus +  "를 만들기 위한 통합된 장바구니 리스트를 만들어줘. " + Ing + "는 이미 집에 있으므로 그 외에 필요한 재료를 나열해줘.";
        requestData["model"] = "text-davinci-003";
        requestData["max_tokens"] = 3000;
        requestData["temperature"] = 0.4;

        auto response = openai::completion().create(requestData);
        //std::cout << "추천 메뉴입니다.\n" << response.dump(2) << '\n';
        BuyList = response["choices"][0]["text"].get<string>();
    }

    void MenuImage(const string& menu){
        json requestData;
        
        
        json requestString;
        requestString["prompt"] = menu + "를 영어로 번역해줘."; // 프롬프트를 생성하고 설정해야 합니다.
        requestString["model"] = "text-davinci-003";
        requestString["max_tokens"] = 500;
        requestString["temperature"] = 0.4;
        auto response = openai::completion().create(requestString);
        string menu_eng = response["choices"][0]["text"].get<string>();
        menu_eng.erase(std::remove_if(menu_eng.begin(), menu_eng.end(), [](unsigned char ch) { return std::isspace(ch); }), menu_eng.end());
        //cout << "공백이 있나요?" << menu_eng << endl;

        requestData["prompt"] = menu_eng;
        requestData["n"] = 1;
        requestData["size"] = "256x256";

        auto image = openai::image().create(requestData);

        auto image_URL = image["data"][0]["url"];


        string sys_cmd = "wget -O ./src/";
        sys_cmd = sys_cmd + menu_eng + ".png ";
        sys_cmd += nlohmann::to_string(image_URL);

        system(sys_cmd.c_str());


        cout << "Image Loding Done " << endl;

        sys_cmd = "tiv -w 100 -h 100 ./src/" + menu_eng + ".png";
        system(sys_cmd.c_str());
    }

    void saveRecipeToFile(int is_List){
        std::ofstream file("식단표.txt");
        if(is_List == 2){
            file << "<<<<<장바구니>>>>>" << std::endl; 

            size_t pos;
            while ((pos = BuyList.find("\n\n")) != std::string::npos) {
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
            for (const auto& line : RecipeArray) {
                file << line << std::endl; // 각 섹션을 파일에 씁니다.
            }
            file.close();
            std::cout << "레시피가 식단표.txt 파일로 저장되었습니다." << std::endl;
        } else {
            std::cout << "파일을 열 수 없습니다." << std::endl;
        }
    }
};

int main() {
        CookingHelper helper;
        string UserIngs, UserMenu, UserAllergy, UserMenuOK, Recommend, s_day, response;

        std::cout << "알러지 등 섭취 시 주의해야 하는 음식을 알려주세요.\n\n" << "음식 : ";
        getline(cin, UserAllergy);
        helper.setAllergy(UserAllergy);

        std::cout << "냉장고에 존재하는 재료를 5가지 이상 알려주세요.\n\n" << "재료 : ";
        getline(cin, UserIngs);
        helper.setIng(UserIngs);
        std::cout<< std::endl;

        Recommend = helper.recomend_menu();

        std::cout << "추천 메뉴입니다.\n" << std::endl;
        helper.print(Recommend);
        std::stringstream ss(Recommend);
        std::string line;
        while (std::getline(ss, line, '\n')) {
            
            //helper.MenuImage(line);
        }
        std::cout << endl;
        //std::cout  << Recommend << endl << endl;


        cout << "다른 메뉴 선택을 원하시나요? (Y/N)\n";
        getline(cin, UserMenuOK);
        std::cout<< std::endl;

        int control = 1;
        while(control == 1){
            if(UserMenuOK == "Y"){
                helper.setExclude(Recommend);
                std::cout << "다른 추천 메뉴입니다." << std::endl; 
                helper.print(helper.recomend_menu());
                std::cout<< std::endl;
                control = 0;
            }
            else if(UserMenuOK == "N"){
                control = 0;
            }
            else{
                std::cout << "(Y/N) 중에 골라 주세요" << std::endl;
                getline(cin, UserMenuOK);
            }
        }

        std::cout << "원하시는 메뉴를 입력해주세요.\n" << "메뉴 : ";
        getline(cin, UserMenu);
        helper.setMenu(UserMenu);
        std::cout << std::endl;

        helper.GenerageRecipe(); 
        helper.print(helper.getRecipe());

        // 식단 추천 기능
        std::cout << "식단 추천 기능을 이용하시겠습니까? (Y/N)" << std::endl;
        getline(cin, response);
        std::cout << std::endl;

        control = 1;
        while(control == 1){
            if(response == "Y"){
                helper.setExclude(UserMenu);

                std::cout << "며칠을 원하나요? (숫자 하나만 입력해주세요. 최대 7일 입니다.)" << std::endl;
                getline(cin, s_day);
                std::cout << std::endl;
                helper.setDays(s_day);
                control = 0;
            }
            else if(response == "N"){
                control = 0;
                break;
            }
            else{
                cout << "(Y/N) 중에 골라 주세요" << endl;
                getline(cin, response);
            }
        }
        std::cout << "식단 추천" << std::endl;
        helper.DietRecommend();
        helper.print(helper.getRecipe());
        std::cout << "식단 추천 완료" << std::endl;

        std::cout << "장바구니 리스트를 보시겠습니까? (Y/N)" << std::endl;
        getline(cin, response);
        int controLL = 1;
        while(controLL == 1){
            if(response == "Y")
            {
                helper.GetList();
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
        helper.print(helper.getBuyList());

        std::cout << "레시피와 장바구니 리스트를 파일로 저장해 드릴까요? (Y/N)" << std::endl << std::endl;
        getline(cin, response);
        int control3 = 1;
        while(control3 == 1){
            if(response == "Y")
            {
                helper.saveRecipeToFile(controLL);
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
    return 0;
}
