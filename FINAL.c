#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <windows.h>
#include <wchar.h>
#include <locale.h>

#define DECK_SIZE 52
#define MAX_HAND_SIZE 11

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"
#define BOLD  "\x1B[1m"
#define CLEAR "\033[H\033[J"

#define CARD_WIDTH 6
#define CARD_SPACING "  "

// ────────────────────────[ SECTION: Types and Definitions ]────────────────────────

typedef enum { CLUBS, DIAMONDS, HEARTS, SPADES } Suit;
typedef enum { TWO = 2, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN, JACK = 11, QUEEN, KING, ACE } Rank;

typedef struct {
    Suit suit;
    Rank rank;
} Card;

typedef struct {
    Card cards[MAX_HAND_SIZE];
    int card_count;
} Hand;

typedef struct {
    Card deck[DECK_SIZE];
    int top_card_index;
} Deck;

const char *suit_symbols[] = { "\u2663", "\u2666", "\u2665", "\u2660" };
const char *suit_colors[] = { WHT, RED, RED, WHT };

// ────────────────────────[ SECTION: Deck Logic ]───────────────────────

const char* get_card_rank_str(int rank) {
    switch (rank) {
        case JACK:  return "J";
        case QUEEN: return "Q";
        case KING:  return "K";
        case ACE:   return "A";
        case TEN:   return "10";
        default: {
            static char buffer[3];
            snprintf(buffer, sizeof(buffer), "%d", rank);
            return buffer;
        }
    }
}

void init_deck(Deck *deck) {
    int i = 0;
    for (int s = CLUBS; s <= SPADES; s++) {
        for (int r = TWO; r <= ACE; r++) {
            deck->deck[i].suit = (Suit)s;
            deck->deck[i].rank = r;
            i++;
        }
    }
    deck->top_card_index = 0;
}

void shuffle_deck(Deck *deck) {
    srand(time(NULL));                                               
    for (int i = DECK_SIZE - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Card temp = deck->deck[i];
        deck->deck[i] = deck->deck[j];
        deck->deck[j] = temp;
    }
}

Card deal_card(Deck *deck) {
    if (deck->top_card_index >= DECK_SIZE) {
        init_deck(deck);
        shuffle_deck(deck);
        deck->top_card_index = 0;
    }
    return deck->deck[deck->top_card_index++];
}

// ────────────────────────[ SECTION: Hand Logic ]────────────────────────

void add_card_to_hand(Hand *hand, Card card) {
    if (hand->card_count < MAX_HAND_SIZE) {
        hand->cards[hand->card_count++] = card;
    }
}

int get_card_value(Card card) {
    if (card.rank >= TEN && card.rank <= KING) return 10;
    if (card.rank == ACE) return 11;
    return card.rank;
}

int calculate_hand_value(Hand *hand) {
    int value = 0;
    int ace_count = 0;
    for (int i = 0; i < hand->card_count; i++) {
        int card_value = get_card_value(hand->cards[i]);
        value += card_value;
        if (hand->cards[i].rank == ACE) ace_count++;
    }
    while (value > 21 && ace_count > 0) {
        value -= 10;
        ace_count--;
    }
    return value;
}

// ────────────────────────[ SECTION: Display Functions ]────────────────────────

void print_card(Card card) {
    const char *rank_str = get_card_rank_str(card.rank);
    printf("%s%s%s%s%s", suit_colors[card.suit], BOLD, rank_str, suit_symbols[card.suit], RESET);
}

void print_hand(Hand *hand, int hide_first_card) {
    if (hand->card_count == 0) return;
    printf("\n");

    for (int i = 0; i < hand->card_count; i++) {
        printf("\u250C");
        for(int j=0; j < CARD_WIDTH; ++j) printf("\u2500");
        printf("\u2510%s", CARD_SPACING);
    }
    printf("\n");

    for (int i = 0; i < hand->card_count; i++) {
        if (i == 0 && hide_first_card) {
            printf("\u2502 ?");
            for(int j=0; j < CARD_WIDTH - 2; ++j) printf(" ");
            printf("\u2502%s", CARD_SPACING);
        } else {
            const char *rank_str = get_card_rank_str(hand->cards[i].rank);
            const char *suit = suit_symbols[hand->cards[i].suit];
            const char *color = suit_colors[hand->cards[i].suit];
            int rank_len = strlen(rank_str);
            int padding = CARD_WIDTH - rank_len - 1;
            if (padding < 0) padding = 0;

            printf("\u2502%s%s%s", color, BOLD, rank_str);
            for(int j=0; j < padding; ++j) printf(" ");
            printf("%s%s\u2502%s", suit, RESET, CARD_SPACING);
        }
    }
    printf("\n");

    for (int line = 0; line < 2; line++) {
        for (int i = 0; i < hand->card_count; i++) {
            printf("\u2502");
            for(int j=0; j < CARD_WIDTH; ++j) printf(" ");
            printf("\u2502%s", CARD_SPACING);
        }
        printf("\n");
    }

    for (int i = 0; i < hand->card_count; i++) {
        printf("\u2514");
        for(int j=0; j < CARD_WIDTH; ++j) printf("\u2500");
        printf("\u2518%s", CARD_SPACING);
    }
    printf("\n");

    if (!hide_first_card) {
        printf("\n%sTotal Value: %s%d%s", CYN, BOLD, calculate_hand_value(hand), RESET);
    }
    printf("\n");
}

// ────────────────────────[ SECTION: Gameplay Logic ]────────────────────────

int player_turn(Deck *deck, Hand *player_hand) {
    char choice[10];
    while (1) {
        print_hand(player_hand, 0);
        int value = calculate_hand_value(player_hand);
        if (value > 21){
            printf("\n%sBUSTED!%s\n", RED, RESET);
            return value;
        }
        if (player_hand->card_count >= MAX_HAND_SIZE) {
            printf("\n%sMaximum hand size reached. Standing automatically.%s\n", YEL, RESET);
            return value;
        }
        printf("\n%sHit or Stand? (h/s): %s", YEL, RESET);
        if (fgets(choice, sizeof(choice), stdin) == NULL) {
            return calculate_hand_value(player_hand);
        }
        choice[strcspn(choice, "\n")] = 0;
        if (strcmp(choice, "h") == 0 || strcmp(choice, "H") == 0) {
            Card card = deal_card(deck);
            printf("\nYou drew: ");
            print_card(card);
            printf("\n");
            add_card_to_hand(player_hand, card);
        } else if (strcmp(choice, "s") == 0 || strcmp(choice, "S") == 0) {
            return calculate_hand_value(player_hand);
        } else {
            printf("Invalid choice. Please enter 'h' or 's'.\n");
        }
    }
    return calculate_hand_value(player_hand);
}

int dealer_turn(Deck *deck, Hand *dealer_hand) {
    printf("\n%sDealer's Turn:%s", MAG, RESET);
    print_hand(dealer_hand, 0);

    while (calculate_hand_value(dealer_hand) < 17) {
        if (dealer_hand->card_count >= MAX_HAND_SIZE) {
            printf("\nDealer reached maximum hand size.\n");
            break;
        }
        printf("\nDealer draws...\n");
        Sleep(1500);

        Card card = deal_card(deck);
        add_card_to_hand(dealer_hand, card);
        print_hand(dealer_hand, 0);
    }

    int value = calculate_hand_value(dealer_hand);
    if (value > 21) {
        printf("\n%sDEALER BUSTS!%s\n", RED, RESET);
    } else {
        printf("\nDealer stands.\n");
    }
    Sleep(1000);
    return value;
}

void print_winner(int player_value, int dealer_value) {
    printf("\n%s----- Final Result -----%s\n", BOLD, RESET);
    printf("%sPlayer's Final Value: %s%d%s\n", BLU, BOLD, player_value, RESET);
    printf("%sDealer's Final Value: %s%d%s\n", MAG, BOLD, dealer_value, RESET);
    printf("\n");

    if (player_value > 21) {
        printf("%sDealer wins! (You busted)%s\n", RED, RESET);
    } else if (dealer_value > 21) {
        printf("%sYOU WIN! (Dealer busted)%s\n", GRN, RESET);
    } else if (player_value > dealer_value) {
        printf("%sYOU WIN!%s\n", GRN, RESET);
    } else if (player_value < dealer_value) {
        printf("%sDealer wins!%s\n", RED, RESET);
    } else {
        printf("%sPush (It's a tie!)%s\n", YEL, RESET);
    }
    printf("\n");
}

// ────────────────────────[ SECTION: Utility Functions ]────────────────────────

void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void print_title() {
    printf(CLEAR);
    printf("%s", BOLD);
    printf("%s   __   _      _               _     _               _            _       __   \n", RED);
    printf("  / /  | |    | |             | |   (_)             | |          | |      \\ \\  \n");
    printf(" / /   | |__  | |  __ _   ___ | | __ _   __ _   ___ | | __       | |__     \\ \\ \n");
    printf("< <    | '_ \\ | | / _` | / __|| |/ /| | / _` | / __|| |/ /       | '_ \\     > >\n");
    printf(" \\ \\   | |_) || || (_| || (__ |   < | || (_| || (__ |   <    _   | | | |   / / \n");
    printf("  \\_\\  |_.__/ |_| \\__,_| \\___||_|\\_\\| | \\__,_| \\___||_|\\_\\  (_)  |_| |_|  /_/  \n");
    printf("                                   _/ |                                        \n");
    printf("                                  |__/                                         \n%s", RESET); 
    printf("\n\n");
}

void clear_screen() {
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD count;
    DWORD cellCount;
    COORD homeCoords = { 0, 0 };

    if (hStdOut == INVALID_HANDLE_VALUE) return;
    if (!GetConsoleScreenBufferInfo(hStdOut, &csbi)) return;
    cellCount = csbi.dwSize.X * csbi.dwSize.Y;
    if (!FillConsoleOutputCharacter(hStdOut, ' ', cellCount, homeCoords, &count)) return;
    if (!FillConsoleOutputAttribute(hStdOut, csbi.wAttributes, cellCount, homeCoords, &count)) return;
    SetConsoleCursorPosition(hStdOut, homeCoords);
}

// ────────────────────────[ SECTION: Main Function ]────────────────────────

int main() {
    SetConsoleOutputCP(CP_UTF8);

    Deck deck;
    init_deck(&deck);
    shuffle_deck(&deck);

    while (1) {
        Hand player_hand = { .card_count = 0 };
        Hand dealer_hand = { .card_count = 0 };

        add_card_to_hand(&player_hand, deal_card(&deck));
        add_card_to_hand(&dealer_hand, deal_card(&deck));
        add_card_to_hand(&player_hand, deal_card(&deck));
        add_card_to_hand(&dealer_hand, deal_card(&deck));

        int player_value = 0, dealer_value = 0;

        while (1) {
            clear_screen();
            print_title();
            printf("%sDealer's Hand:%s", MAG, RESET);
            print_hand(&dealer_hand, player_value == 0);
            printf("\n%sYour Hand:%s", BLU, RESET);
            print_hand(&player_hand, 0);

            player_value = calculate_hand_value(&player_hand);
            if (player_value > 21) {
                printf("\n%sBUSTED!%s\n", RED, RESET);
                break;
            }

            if (player_value == 0) break;

            char choice[10];
            printf("\n%sHit or Stand? (h/s): %s", YEL, RESET);
            if (fgets(choice, sizeof(choice), stdin) == NULL || (choice[0] != 'h' && choice[0] != 'H' && choice[0] != 's' && choice[0] != 'S')) {
                continue;
            }

            if (choice[0] == 'h' || choice[0] == 'H') {
                add_card_to_hand(&player_hand, deal_card(&deck));
            } else {
                break;
            }
        }

        if (player_value <= 21) {
            dealer_value = dealer_turn(&deck, &dealer_hand);
        }

        clear_screen();
        print_title();
        printf("%sDealer's Hand:%s", MAG, RESET);
        print_hand(&dealer_hand, 0);
        printf("\n%sYour Hand:%s", BLU, RESET);
        print_hand(&player_hand, 0);
        print_winner(player_value, dealer_value);

        char play_again[10];
        printf("%sPlay again? (y/n): %s", YEL, RESET);
        if (fgets(play_again, sizeof(play_again), stdin) == NULL || (play_again[0] != 'y' && play_again[0] != 'Y')) {
            break;
        }

        init_deck(&deck);
        shuffle_deck(&deck);
    }

    printf("\n%sThanks for playing!%s\n\n", CYN, RESET);
    return 0;
}
