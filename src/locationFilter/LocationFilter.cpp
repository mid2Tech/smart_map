#include "LocationFilter.h"

#define FILTER_SIZE 3 // Giảm từ 5 xuống 3 để nhận diện trạm nhanh hơn
int location_history[FILTER_SIZE] = {0, 0, 0};
int history_idx = 0;

int get_filtered_location(int raw_loc)
{
    if (raw_loc > 0)
    {
        location_history[history_idx] = raw_loc;
        history_idx = (history_idx + 1) % FILTER_SIZE;
    }
    int counts[5] = {0};
    for (int i = 0; i < FILTER_SIZE; i++)
    {
        if (location_history[i] > 0)
            counts[location_history[i]]++;
    }
    int best_loc = 0;
    int max_count = 0;
    for (int i = 1; i <= 4; i++)
    {
        if (counts[i] > max_count)
        {
            max_count = counts[i];
            best_loc = i;
        }
    }
    static int last_stable = 0;
    if (max_count >= 2) // Chỉ cần 2 lần đọc trúng là nhảy số ngay
        last_stable = best_loc;

    return last_stable;
}