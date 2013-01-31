#include <iostream>

using namespace std;

enum days_t {
    MONDAY,
    TUESDAY,
    WEDNESDAY,
    THURDAY,
    FRIDAY,
    SATURDAY,
    SUNDAY
};

int main()
{
    days_t day = MONDAY;
    
    
    for(int i = 0; i < 7; i++)
    {
        switch (day){-
            case MONDAY:
                cout << "Its monday" << "\n";
                day = TUESDAY;
                break;
            case TUESDAY:
                cout << "Its tuesday" << "\n";
                day = MONDAY;
                break;               
            // and so on.....
        }
    }

    return 0;
}
