#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

typedef struct
{
    int length;
    int attempts;
} Answers;

const char *words[10] =
        {
                "example",
                "hangman",
                "programming",
                "computer",
                "science",
                "random",
                "words",
                "array",
                "selection",
                "game"
        };

typedef enum
{
    false,
    true
}bool;

void processGuess(char guess, const char *word, char *hiddenWord, int length, bool *isCorrect, int *attempts, char *inputArray)
{
    if (guess >= 97 && guess <= 122) // ASCII values for lowercase letters
    {
        // strchr returns a pointer to the first occurrence of the character in the string
        if (strchr(inputArray, guess) != NULL) // Check if the letter has already been guessed
        {
            printf("You have already guessed this letter. Please try again.\n");
            return;
        }
        strncat(inputArray, &guess, 1); // Append the guessed letter to the inputArray

        bool foundLetter = false;
        for (int i = 0; i < length; i++)
        {
            if (guess == word[i])
            {
                hiddenWord[i] = guess;
                foundLetter = true;
            }
        }

        if (foundLetter)
        {
            printf("Correct guess!\n");
            if (strcmp(hiddenWord, word) == 0) //strcmp returns 0 if the strings are equal
            {
                *isCorrect = true;
            }
        }
        else
        {
            printf("Wrong guess!\n");
            (*attempts)--;
        }
        printf("Word: %s\n", hiddenWord);
        printf("Attempts left: %d\n", *attempts);
    }
    else
    {
        printf("Please enter a lowercase letter (a-z)\n");
    }
}

int main(void)
{
    Answers answer;
    bool isCorrect = false;

    srand(time(NULL));
    int randomIndex = rand() % 10;

    answer.length = strlen(words[randomIndex]);
    answer.attempts = 7;
    char hiddenWord[answer.length + 1];  // +1 for null terminator
    memset(hiddenWord, '_', answer.length);
    hiddenWord[answer.length] = '\0';

    printf("Selected word: %s <--- FOR TESTING\n", words[randomIndex]); // Testing

    printf("Welcome to the Hangman Game!\n");
    printf("You have %d attempts to guess the word.\n", answer.attempts);
    printf("The word has %d letters.\n", answer.length);
    printf("%s\n", hiddenWord);

    char input[2];  // one char and a null terminator
    char inputArray[answer.length + 1]; // Declare inputArray
    memset(inputArray, 0, sizeof(inputArray)); // Initialize inputArray to an empty string

    while(isCorrect == false && answer.attempts > 0)
    {
        printf("Letters guessed: %s\n", inputArray); // Print the letters guessed so far
        printf("Enter a letter: ");
        scanf(" %c", &input[0]);

        while (getchar() != '\n') // Check for extra input
        {
            fflush(stdin); //stdin == Standard Input
            printf("Warning: Extra input detected. Please enter only one character.\n");
            printf("Enter a letter: ");
            scanf(" %c", &input[0]);
        }

        processGuess(input[0], words[randomIndex], hiddenWord, answer.length, &isCorrect, &answer.attempts, inputArray);
    }

    if (isCorrect)
    {
        printf("\nCongratulations! You won! The word was: %s\n", words[randomIndex]);
    }
    else
    {
        printf("\nGame Over! You ran out of attempts. The word was: %s\n", words[randomIndex]);
    }

    return 0;
}