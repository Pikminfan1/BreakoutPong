#pragma once

enum class GameStatus
{
    Menu,
    Playing,
    GameOver,
    Won
};

class GameStateManager
{
private:
    int lives = 3;
    float score = 0.0f;

    GameStatus status = GameStatus::Menu;

public:
    int GetLives() const { return lives; }
    float GetScore() const { return score; }
    GameStatus GetStatus() const { return status; }

    void StartGame() { status = GameStatus::Playing; }
    void WinGame() { status = GameStatus::Won; }
    void LoseLife()
    {
        if (lives > 0)
            --lives;
        if (lives == 0)
            status = GameStatus::GameOver;
    }
    void SetLives(int newLives) { lives = newLives; }
    void SetScore(float newScore) { score = newScore; }
    void AddScore(float delta) { score += delta; }
    void Reset()
    {
        lives = 3;
        score = 0.0f;
        status = GameStatus::Menu;
    }
    void SetStatus(GameStatus newStatus) { status = newStatus; }
};
