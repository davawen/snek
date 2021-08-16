#include <chrono>
#include <iostream>
#include <mutex>
#include <vector>
#include <thread>
#include <string>

#include <ncurses.h>
#include <time.h>

#ifdef _WIN32
#include <Windows.h>
#endif
#ifdef linux
#include <unistd.h>
#endif

constexpr int width = 10;
constexpr int height = 10;
constexpr int indexLength = width * height;

std::mutex mutex;

typedef struct
{
	std::vector<int>  *playerPos;
	int               *playerSpeed;
	int               *applePos;
	
	bool               pressedKey; // Allow more responsible input
	bool               finished;
} DATA;

DATA g_data;

void constructGrid(std::string &display)
{
	// Construct grid, then transform it to string representation
	// Very lazy approach, but ¯\_(ツ)_/¯
	
	int *grid = new int[indexLength];
	
	for(int i = 0; i < indexLength; i++)
		grid[i] = 0;
	
	// mutex.lock(); No need to lock when playerSpeed isn't read
	
	for(auto it = g_data.playerPos->begin()+1; it != g_data.playerPos->end(); it++)
	{
		grid[*it] = 2;
	}
	
	grid[ g_data.playerPos->front() ] = 1; // Differentiate head and other parts
	
	if(*g_data.applePos != -1) grid[*g_data.applePos] = 3;
	
	// mutex.unlock();
	
	display.clear();
	
	for(int j = 0; j < height; j++)
	{
		for(int i = 0; i < width; i++)
		{
			int index = j * width + i;
			
			switch(grid[index])
			{
				case 0:
					display += '`';
					break;
				case 1:
					display += '@';
					break;
				case 2:
					display += '#';
					break;
				case 3:
					display += 'O';
					break;
			}
			
			display += " ";
		}
		
		display += '\n';
	}
	
	delete[] grid;
}

void setApplePos()
{
	int newApplePos = rand() % indexLength;
	
	for(int i = 0; i < g_data.playerPos->size(); i++)
	{
		if(newApplePos == (*g_data.playerPos)[i])
		{
			newApplePos = rand() % indexLength;
			i = 0;
		}
	}
	
	*g_data.applePos = newApplePos;
}

void inputControl()
{
	nodelay(stdscr, true);
	
	while(!g_data.finished)
	{
		char ch;
		
		if((ch = getch()) == -1) continue;
		
		std::lock_guard<std::mutex> lock(mutex);
		
		switch(ch)
		{
			case 65: // UP
				*g_data.playerSpeed = -width;
				g_data.pressedKey = true;
				break;
			case 66: // DOWN
				*g_data.playerSpeed = width;
				g_data.pressedKey = true;
				break;
			case 67: // RIGHT
				*g_data.playerSpeed = 1;
				g_data.pressedKey = true;
				break;
			case 68: // LEFT
				*g_data.playerSpeed = -1;
				g_data.pressedKey = true;
				break;
			case 'q':
				g_data.finished = true;
				g_data.pressedKey = true;
				break;
		}
	}
}

long now()
{
	return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

int main(int argc, char **argv)
{
	initscr();
	raw();
	noecho();
	
	// x and y are converted to indices in a one dimensional array
	// index = y * width + x;
	// x, y = index % width, index / (int)width;
	
	std::vector<int> playerPos = { 45 };
	int playerSpeed = 0; // Don't move by default
	
	int applePos = -1;
	
	g_data.playerPos = &playerPos;
	g_data.playerSpeed = &playerSpeed;
	g_data.applePos = &applePos;
	g_data.finished = false;
	
	setApplePos();
	
	std::string display;
	
	std::thread inputThread(inputControl);
	
	while(true)
	{
		constructGrid(display);
		mvaddstr(0, 0, display.c_str());
		
		// Wait a half second
		long startTime = now();
		long currentTime;
		
		while((currentTime = now()) - startTime < 250'000)
		{
			if(currentTime - startTime > 50'000 && g_data.pressedKey) break;
		}
		
		std::unique_lock<std::mutex> lock(mutex, std::try_to_lock);
		
		g_data.pressedKey = false;
		
		int newPos = (*g_data.playerPos)[0] + *g_data.playerSpeed;
		bool lost = false;
		
		// Shift all positions by one
		for(size_t i = g_data.playerPos->size(); i > 0; i--)
		{
			if(playerSpeed != 0 && newPos == (*g_data.playerPos)[i]) lost = true;
			
			(*g_data.playerPos)[i] = (*g_data.playerPos)[i - 1];
		}
		
		if(g_data.finished) break;
		
		g_data.playerPos->front() = newPos;
		
		int oldPos = (*g_data.playerPos)[1];
		
		// Check for out of bounds
		if( lost ||
			newPos < 0 ||
			newPos > indexLength-1 ||
			(oldPos % width == width-1 && newPos % width == 0      ) ||
			(oldPos % width == 0       && newPos % width == width-1)
		)
		{
			addstr("You lost!!");
			
			g_data.finished = true;
			
			lock.unlock();
			
			sleep(1);
			break;
		}
		else if(newPos == applePos)
		{
			g_data.playerPos->push_back(abs(g_data.playerPos->back() - *g_data.playerSpeed));
			
			setApplePos();
		}
	}
	
	inputThread.join();
	
	endwin();
	
	return 0;
}