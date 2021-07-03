#include "sff_sudoku_solver.h"

#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

std::string readFile(fs::path path)
{
	// Open the stream to 'lock' the file.
	std::ifstream f(path, std::ios::in | std::ios::binary);
	const bool   is_good = f.good();
	if (is_good) {
		// Obtain the size of the file.
		const auto sz = fs::file_size(path);

		// Create a buffer.
		std::string result(sz, '\0');

		// Read the whole file into the buffer.
		f.read(result.data(), sz);
		return result;
	} else {
		return std::string{ "" };
	}
}

int main()
{
	fs::path p = fs::current_path();
	std::cout << p << '\n';

	std::string puzzle_buf = readFile(p.parent_path() / "hard_sudokus.txt");
	std::cout << puzzle_buf << '\n';


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
	//we're missing 2 7 9 which means there are several variations
	/*
		. . . | . . 5 | . 8 .
		. . . | 6 . 1 | . 4 3
		. . . | . . . | . . .
		------ + ------ + ------
		. 1 . | 5 . . | . . .
		. . . | 1 . 6 | . . .
		3 . . | . . . | . . 5
		------ + ------ + ------
		5 3 . | . . . | . 6 1
		. . . | . . . | . . 4
		. . . | . . . | . . .
	*/
	std::vector<char> puzzle_nuke = {
		'.', '.', '.', '.', '.', '5', '.', '8', '.',
		'.', '.', '.', '6', '.', '1', '.', '4', '3',
		'.', '.', '.', '.', '.', '.', '.', '.', '.',
		'.', '1', '.', '5', '.', '.', '.', '.', '.',
		'.', '.', '.', '1', '.', '6', '.', '.', '.',
		'3', '.', '.', '.', '.', '.', '.', '.', '5',
		'5', '3', '.', '.', '.', '.', '2', '6', '1',
		'.', '.', '.', '.', '.', '.', '.', '.', '4',
		'.', '.', '.', '.', '.', '.', '.', '.', '.'
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

	auto ts_n0 = std::chrono::steady_clock::now();
	//solver.solveSudoku(puzzle);
	//solver.solveSudoku4(puzzle_arr);
	//solver.solveSudoku3(puzzle);
	/*
	size_t guesses_impossible;
	sff_solver(puzzle_nuke.data(), puzzle_nuke.size(), 0, solution_vec.data(), &guesses_impossible);
	auto ts_n1 = std::chrono::steady_clock::now();

	for (size_t xy = 0; xy < puzzle_vec.size(); xy++) {
		std::cout << solution_vec.data()[xy] << ' ';
		if ((xy % puzzle_len) == (puzzle_len - 1))
			std::cout << '\n';
	}
	std::cout << '\n';
	*/
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
	size_t guesses3;
	sff_solver_simple(puzzle_vec.data(), puzzle_vec.size(), 0, solution_vec.data(), &guesses3);
	//solver.solveSudoku4(puzzle_arr);
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
	size_t width = 0;
	for (; width < puzzle_buf.size() && !(puzzle_buf[width] == '\r' || puzzle_buf[width] == '\n'); width++) {
	}
	size_t width2 = width;
	for (; width2 < puzzle_buf.size() && (puzzle_buf[width2] == '\r' || puzzle_buf[width2] == '\n'); width2++) {
	}

	bool solved = true;
	size_t hard_ns = 0;
	size_t guess_count = 0;
	auto ts_6 = std::chrono::steady_clock::now();
	for (size_t i = 0; i < puzzle_buf.size(); i += width2) {
		//std::cout << std::string_view{ puzzle_buf.data() + i, 81 } << '\n';
		auto ts_8 = std::chrono::steady_clock::now();
		//solved &= (bool)sff_solver(puzzle_buf.data()+i, width, 0, solution_vec.data(), &guesses);
		//solved &= (bool)sff_solver_simple(puzzle_buf.data() + i, width, 0, solution_vec.data(), &guesses);
		solved &= (bool)solver.solveSudokuN<81>(puzzle_buf.data() + i, width, 0, solution_vec.data(), &guesses);
		//solved &= (bool)solver.solveSudokuNCrossHatch<81>(puzzle_buf.data() + i, width, 0, solution_vec.data(), &guesses);
		//solved &= (bool)solver.solveSudokuNFill<81>(puzzle_buf.data() + i, width, 0, solution_vec.data(), &guesses);
		auto ts_9 = std::chrono::steady_clock::now();
		guess_count += guesses;
		hard_ns += (ts_9 - ts_8).count();

		std::cout << "guesses\n";
		std::cout << guesses << '\n';
		//std::cout << std::string_view{solution_vec.data(), width} << '\n';

		for (size_t xy = 0; xy < puzzle_vec.size(); xy++) {
			char tail = ((xy % puzzle_len) == (puzzle_len - 1)) ? '\n' : ' ';
			std::cout << solution_vec.data()[xy] << tail;
		}
		
		std::cout << '\n';

	}
	auto ts_7 = std::chrono::steady_clock::now();
#if 0
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
#endif
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

	std::cout << "_____________\n";

	ns = (ts_5 - ts_4).count();
	std::cout << "time (ns)\n";
	std::cout << ns << '\n';

	std::cout << "time (us)\n";
	std::cout << (double)ns / (double)1e3 << '\n';

	std::cout << "time (ms)\n";
	std::cout << (double)ns / (double)1e6 << '\n';


	std::cout << "_____________\n";

	ns = (ts_7 - ts_6).count();
	std::cout << "time (ns)\n";
	std::cout << ns << '\n';

	std::cout << "time (us)\n";
	std::cout << (double)ns / (double)1e3 << '\n';

	std::cout << "time (ms)\n";
	std::cout << (double)ns / (double)1e6 << '\n';

	//hard_ns

	std::cout << "_____________\n";

	std::cout << "time (ns)\n";
	std::cout << hard_ns << '\n';

	std::cout << "time (us)\n";
	std::cout << (double)hard_ns / (double)1e3 << '\n';

	std::cout << "time (ms)\n";
	std::cout << (double)hard_ns / (double)1e6 << '\n';

	size_t puzzle_count = puzzle_buf.size() / width2;
	std::cout << "puzzles\n";
	std::cout << puzzle_count << '\n';

	std::cout << "guesses\n";
	std::cout << guess_count << '\n';

	std::cout << "guesses (per puzzle)\n";
	std::cout << (guess_count / puzzle_count) << '\n';

	std::cout << "time (ns)\n";
	std::cout << (hard_ns / puzzle_count) << '\n';

	std::cout << "time (us)\n";
	std::cout << (double)(hard_ns / puzzle_count) / (double)1e3 << '\n';

	std::cout << "time (ms)\n";
	std::cout << (double)(hard_ns / puzzle_count) / (double)1e6 << '\n';

	return 0;
}
