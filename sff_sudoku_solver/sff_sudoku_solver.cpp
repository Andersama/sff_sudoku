#include "sff_sudoku_solver.h"

int main()
{
	std::vector<std::vector<char>> puzzle = 
	{
		{'5', '3', '.', '.', '7', '.', '.', '.', '.'},
		{'6', '.', '.', '1', '9', '5', '.', '.', '.'},
		{'.', '9', '8', '.', '.', '.', '.', '6', '.'},
		{'8', '.', '.', '.', '6', '.', '.', '.', '3'},
		{'4', '.', '.', '8', '.', '3', '.', '.', '1'},
		{'7', '.', '.', '.', '2', '.', '.', '.', '6'},
		{'.', '6', '.', '.', '.', '.', '2', '8', '.'},
		{'.', '.', '.', '4', '1', '9', '.', '.', '5'},
		{'.', '.', '.', '.', '8', '.', '.', '7', '9'}
	};
	std::vector<char> puzzle_vec = {
		'5', '3', '.', '.', '7', '.', '.', '.', '.',
		'6', '.', '.', '1', '9', '5', '.', '.', '.',
		'.', '9', '8', '.', '.', '.', '.', '6', '.',
		'8', '.', '.', '.', '6', '.', '.', '.', '3',
		'4', '.', '.', '8', '.', '3', '.', '.', '1',
		'7', '.', '.', '.', '2', '.', '.', '.', '6',
		'.', '6', '.', '.', '.', '.', '2', '8', '.',
		'.', '.', '.', '4', '1', '9', '.', '.', '5',
		'.', '.', '.', '.', '8', '.', '.', '7', '9'
	};
	std::vector<char> solution_vec;
	solution_vec.reserve(puzzle_vec.size());

/*
	for (size_t y = 0; y < puzzle.size(); y++) {
		for (size_t x = 0; x < puzzle[0].size(); x++) {
			std::cout << puzzle[y][x] << ' ';
		}
		std::cout << '\n';
	}
*/
	/*
	std::array<std::array<char,9>,9> puzzle_arr = 
	{
		std::array<char,9>{'5', '3', '.', '.', '7', '.', '.', '.', '.'},
		std::array<char,9>{'6', '.', '.', '1', '9', '5', '.', '.', '.'},
		std::array<char,9>{'.', '9', '8', '.', '.', '.', '.', '6', '.'},
		std::array<char,9>{'8', '.', '.', '.', '6', '.', '.', '.', '3'},
		std::array<char,9>{'4', '.', '.', '8', '.', '3', '.', '.', '1'},
		std::array<char,9>{'7', '.', '.', '.', '2', '.', '.', '.', '6'},
		std::array<char,9>{'.', '6', '.', '.', '.', '.', '2', '8', '.'},
		std::array<char,9>{'.', '.', '.', '4', '1', '9', '.', '.', '5'},
		std::array<char,9>{'.', '.', '.', '.', '8', '.', '.', '7', '9'}
	};
	*/
	std::array<std::array<char, 9>, 9> puzzle_arr =
	{
		std::array<char,9>{'.', '.', '.', '4', '.', '.', '9', '.', '.'},
		std::array<char,9>{'.', '.', '.', '8', '.', '.', '.', '1', '2'},
		std::array<char,9>{'.', '.', '.', '.', '1', '.', '6', '.', '.'},
		std::array<char,9>{'.', '.', '.', '.', '.', '.', '.', '8', '.'},
		std::array<char,9>{'1', '.', '2', '3', '.', '.', '.', '5', '.'},
		std::array<char,9>{'.', '7', '4', '.', '9', '.', '1', '.', '.'},
		std::array<char,9>{'.', '.', '8', '2', '.', '4', '.', '.', '5'},
		std::array<char,9>{'.', '4', '.', '.', '.', '.', '.', '.', '.'},
		std::array<char,9>{'.', '6', '.', '.', '.', '.', '.', '3', '.'}
	};
	/*
	for (size_t y = 0; y < puzzle_arr.size(); y++) {
		for (size_t x = 0; x < puzzle_arr[0].size(); x++) {
			std::cout << puzzle_arr[y][x] << ' ';
		}
		std::cout << '\n';
	}
	*/
	size_t puzzle_len = std::sqrt(puzzle_vec.size());
	for (size_t xy = 0; xy < puzzle_vec.size(); xy++) {
		std::cout << puzzle_vec.data()[xy] << ' ';
		if ((xy % puzzle_len) == (puzzle_len-1))
			std::cout << '\n';
	}
	std::cout << '\n';

	Solver solver;
	auto ts_0 = std::chrono::steady_clock::now();
	//solver.solveSudoku(puzzle);
	//solver.solveSudoku4(puzzle_arr);
	//solver.solveSudoku3(puzzle);
	size_t guesses;
	sff_solver(puzzle_vec.data(), puzzle_vec.size(), 0, solution_vec.data(), &guesses);
	auto ts_1 = std::chrono::steady_clock::now();

	//size_t puzzle_out_len = std::sqrt(solution_vec.size());
	for (size_t xy = 0; xy < puzzle_vec.size(); xy++) {
		std::cout << solution_vec.data()[xy] << ' ';
		if ((xy % puzzle_len) == (puzzle_len -1))
			std::cout << '\n';
	}
	std::cout << '\n';

	auto ts_2 = std::chrono::steady_clock::now();
	size_t guesses2;
	sff_solver_simple(puzzle_vec.data(), puzzle_vec.size(), 0, solution_vec.data(), &guesses2);
	auto ts_3 = std::chrono::steady_clock::now();

	std::cout << '\n';

	auto ts_4 = std::chrono::steady_clock::now();
	solver.solveSudoku4(puzzle_arr);
	auto ts_5 = std::chrono::steady_clock::now();

	std::cout << '\n';
	/*
	for (size_t y = 0; y < puzzle_arr.size(); y++) {
		for (size_t x = 0; x < puzzle_arr[0].size(); x++) {
			std::cout << puzzle_arr[y][x] << ' ';
		}
		std::cout << '\n';
	}
	*/
	for (size_t n = 0; n < 50000; n++) {
		/*
		puzzle =
		{
			{'5', '3', '.', '.', '7', '.', '.', '.', '.'},
			{'6', '.', '.', '1', '9', '5', '.', '.', '.'},
			{'.', '9', '8', '.', '.', '.', '.', '6', '.'},
			{'8', '.', '.', '.', '6', '.', '.', '.', '3'},
			{'4', '.', '.', '8', '.', '3', '.', '.', '1'},
			{'7', '.', '.', '.', '2', '.', '.', '.', '6'},
			{'.', '6', '.', '.', '.', '.', '2', '8', '.'},
			{'.', '.', '.', '4', '1', '9', '.', '.', '5'},
			{'.', '.', '.', '.', '8', '.', '.', '7', '9'}
		};
		solver.solveSudoku(puzzle);
		*/
		/*
		puzzle_arr = {
			std::array<char,9>{'5', '3', '.', '.', '7', '.', '.', '.', '.'},
			std::array<char,9>{'6', '.', '.', '1', '9', '5', '.', '.', '.'},
			std::array<char,9>{'.', '9', '8', '.', '.', '.', '.', '6', '.'},
			std::array<char,9>{'8', '.', '.', '.', '6', '.', '.', '.', '3'},
			std::array<char,9>{'4', '.', '.', '8', '.', '3', '.', '.', '1'},
			std::array<char,9>{'7', '.', '.', '.', '2', '.', '.', '.', '6'},
			std::array<char,9>{'.', '6', '.', '.', '.', '.', '2', '8', '.'},
			std::array<char,9>{'.', '.', '.', '4', '1', '9', '.', '.', '5'},
			std::array<char,9>{'.', '.', '.', '.', '8', '.', '.', '7', '9'}
		};
		solver.solveSudoku4(puzzle_arr);
		*/
		sff_solver(puzzle_vec.data(), puzzle_vec.size(), 0, solution_vec.data(), &guesses);
	}

	size_t ns = (ts_1 - ts_0).count();

	std::cout << "time (ns)\n";
	std::cout << ns << '\n';

	std::cout << "time (us)\n";
	std::cout << (double)ns / (double)1e3 << '\n';

	std::cout << "time (ms)\n";
	std::cout << (double)ns / (double)1e6 << '\n';

	std::cout << "guesses\n";
	std::cout << guesses << '\n';

	std::cout << "_____________\n";

	ns = (ts_3 - ts_2).count();
	std::cout << "time (ns)\n";
	std::cout << ns << '\n';

	std::cout << "time (us)\n";
	std::cout << (double)ns / (double)1e3 << '\n';

	std::cout << "time (ms)\n";
	std::cout << (double)ns / (double)1e6 << '\n';
	
	std::cout << "guesses\n";
	std::cout << guesses2 << '\n';

	ns = (ts_5 - ts_4).count();
	std::cout << "time (ns)\n";
	std::cout << ns << '\n';

	std::cout << "time (us)\n";
	std::cout << (double)ns / (double)1e3 << '\n';

	std::cout << "time (ms)\n";
	std::cout << (double)ns / (double)1e6 << '\n';

	return 0;
}
