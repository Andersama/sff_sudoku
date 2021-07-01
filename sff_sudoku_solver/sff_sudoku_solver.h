#pragma once
#include <cstdint>
#include <vector>
#include <array>
#include <algorithm>
#include <iostream>
#include <chrono>

#include <bitset>

#include <limits>
// pretty sure this is msvc only...we need popcount
#include <intrin.h>

namespace Detail
{
	double constexpr sqrtNewtonRaphson(double x, double curr, double prev)
	{
		return curr == prev
			? curr
			: sqrtNewtonRaphson(x, 0.5 * (curr + x / curr), curr);
	}
}

/*
* Constexpr version of the square root
* Return value:
*   - For a finite and non-negative value of "x", returns an approximation for the square root of "x"
*   - Otherwise, returns NaN
*/
namespace patchwork {
	double constexpr sqrt(double x)
	{
		return x >= 0 && x < std::numeric_limits<double>::infinity()
			? Detail::sqrtNewtonRaphson(x, x, 0)
			: std::numeric_limits<double>::quiet_NaN();
	}
}
/*
constexpr std::array<uint8_t, 256> char_to_digit = []() {
	std::array<uint8_t, 256> table;
	for (size_t i = 0; i < table.size(); i++)
		table[i] = 255;

	for (char c = '0'; c <= '9'; c++)
		table[(uint8_t)c] = (uint8_t)c - (uint8_t)'0';

	return table;
}();
*/
constexpr uint32_t calculate_block(uint32_t x, uint32_t y, uint32_t w) noexcept {
	return (y / w) * w + (x / w);
}

constexpr uint32_t calculate_block_idx(uint32_t x, uint32_t y, uint32_t w) noexcept {
	uint32_t x2 = x % w;
	uint32_t y2 = y % w;
	return (y2 * w) + x2;
}

class Selection {
public:
	uint8_t value = 0;
	uint8_t row = 0;
	uint8_t col = 0;
	uint8_t blk = 0;

	Selection(uint8_t v, uint8_t r, uint8_t c, uint8_t b) : value(v), row(r), col(c), blk(b)
	{};
	Selection() = default;
};

class SelectionInfo {
public:
	uint8_t value = 0;
	uint8_t row = 0;
	uint8_t col = 0;
	uint8_t blk = 0;
	uint8_t blk_idx = 0;

	SelectionInfo(uint8_t v, uint8_t r, uint8_t c, uint8_t b, uint8_t bi) : value(v), row(r), col(c), blk(b), blk_idx(bi)
	{};
	SelectionInfo() = default;
};

class Solver {
public:
	void solveSudoku(std::vector<std::vector<char>>& board) {
		//ignore weird boards
		if (!board.size())
			return;
		if (board[0].size() != board.size())
			return;

		std::vector<Selection> selections;
		uint32_t board_len = board.size();
		uint32_t blk_len = std::sqrt(board_len);
		uint32_t cache_width = board_len + 1;
		selections.reserve(board_len * board_len);

		std::vector<uint8_t> selection_cache;
		uint32_t cache_size = (board_len * cache_width);
		uint32_t board_size = board_len * board_len;

		selection_cache.reserve(3 * cache_size + board_size);
		for (size_t i = 0; i < (3 * cache_size + board_size); i++) {
			selection_cache.emplace_back(0);
		}

		for (size_t i = 0; i < board_size; i++) {
			selections.emplace_back((uint8_t)0, (uint8_t)0, (uint8_t)0, (uint8_t)0);
		}

		uint32_t known_locations = 0;
		uint32_t tmp_idx = 0;
		uint32_t back_idx = board_size - 1;
		for (size_t y = 0; y < board.size(); y++) {
			for (size_t x = 0; x < board.size(); x++) {
				uint8_t digit = (uint8_t)board[y][x] - (uint8_t)'0';
				//char_to_digit[board[y][x]];
				if (digit <= board.size()) {
					known_locations++;
					uint8_t b = calculate_block(x, y, blk_len);

					uint8_t* y_cached = &selection_cache[(0 * cache_size) + (y * (cache_width)) + digit];
					uint8_t* x_cached = &selection_cache[(1 * cache_size) + (x * (cache_width)) + digit];
					uint8_t* b_cached = &selection_cache[(2 * cache_size) + (b * (cache_width)) + digit];
					if (*y_cached || *x_cached || *b_cached) {
						//bad start to the puzzle (impossible to solve b/c invalid)
						std::cout << "already marked\n";
						return;
					}
					//selections.emplace_back((uint8_t)digit, (uint8_t)y, (uint8_t)x, (uint8_t)b);
					selections[tmp_idx].value = digit;
					selections[tmp_idx].row = y;
					selections[tmp_idx].col = x;
					selections[tmp_idx].blk = b;
					tmp_idx++;

					*y_cached = 1;
					*x_cached = 1;
					*b_cached = 1;
				}
				else {
					//fill in w/ 0 for unselected
					uint8_t b = calculate_block(x, y, blk_len);
					selections[back_idx].value = 0;
					selections[back_idx].row = y;
					selections[back_idx].col = x;
					selections[back_idx].blk = b;
					back_idx--;
					//selections.emplace_back((uint8_t)0, (uint8_t)y, (uint8_t)x, (uint8_t)b);

				}
			}
		}
		//bring all prefilled to front
		std::partition(selections.begin(), selections.end(), [](Selection& lhs) {
			return lhs.value > 0;
			});
		//sort all unknown values by # of choices available lower -> better, ergo more marks -> better
		/*
		update counts at locations once
		*/
		for (size_t idx = known_locations; idx < selections.size(); idx++) {
			uint8_t count = 0;
			Selection pick = selections[idx];
			uint8_t* y_pick = &selection_cache[(0 * cache_size) + (pick.row * (cache_width))];
			uint8_t* x_pick = &selection_cache[(1 * cache_size) + (pick.col * (cache_width))];
			uint8_t* b_pick = &selection_cache[(2 * cache_size) + (pick.blk * (cache_width))];
			for (uint32_t digit = 1; digit < cache_width; digit++) {
				count += (y_pick[digit] || x_pick[digit] || b_pick[digit]);
			}
			uint8_t* c_pick = &selection_cache[(3 * cache_size) + (pick.row * board_len) + pick.col];
			*c_pick = count;
		}
		auto it = std::min_element(selections.begin() + known_locations, selections.end(), [&selection_cache, cache_size, board_len](Selection& lhs, Selection& rhs) {
			size_t lhs_marks = 0;
			size_t rhs_marks = 0;
			lhs_marks = selection_cache[(3 * cache_size) + (lhs.row * board_len) + lhs.col];
			rhs_marks = selection_cache[(3 * cache_size) + (rhs.row * board_len) + rhs.col];

			return rhs_marks < lhs_marks;
			});
		if (known_locations < selections.size()) {
			std::swap(selections[known_locations], *it);
		}

		size_t index = known_locations;
		//do the solve
		while (index >= known_locations && index < selections.size()) {
			Selection& pick = selections[index];

			for (size_t digit = pick.value; ++digit <= board_len;) {
				uint8_t* y_cached = &selection_cache[(0 * cache_size) + ((uint32_t)pick.row * (cache_width)) + digit];
				uint8_t* x_cached = &selection_cache[(1 * cache_size) + ((uint32_t)pick.col * (cache_width)) + digit];
				uint8_t* b_cached = &selection_cache[(2 * cache_size) + ((uint32_t)pick.blk * (cache_width)) + digit];

				if (*y_cached || *x_cached || *b_cached) {
					//skip things we cannot use be b/c of other selections
					continue;
				}

				//remark cell with new selection
				uint8_t* y_prev = &selection_cache[(0 * cache_size) + ((uint32_t)pick.row * (cache_width)) + pick.value];
				uint8_t* x_prev = &selection_cache[(1 * cache_size) + ((uint32_t)pick.col * (cache_width)) + pick.value];
				uint8_t* b_prev = &selection_cache[(2 * cache_size) + ((uint32_t)pick.blk * (cache_width)) + pick.value];
				*y_prev = 0;
				*x_prev = 0;
				*b_prev = 0;
				*y_cached = 1;
				*x_cached = 1;
				*b_cached = 1;
				pick.value = digit;

				//break;
				goto next_stack;
			}
			// failed to fill particular cell
			--index;
			if (index >= known_locations && index < selections.size()) {
				// unmark cell
				uint8_t* y_prev = &selection_cache[(0 * cache_size) + ((uint32_t)pick.row * (cache_width)) + pick.value];
				uint8_t* x_prev = &selection_cache[(1 * cache_size) + ((uint32_t)pick.col * (cache_width)) + pick.value];
				uint8_t* b_prev = &selection_cache[(2 * cache_size) + ((uint32_t)pick.blk * (cache_width)) + pick.value];
				*y_prev = 0;
				*x_prev = 0;
				*b_prev = 0;
				pick.value = 0;
				continue;
			}
			else {
				// we walked back and undid the entire puzzle (impossible starting condition)
				std::cout << "impossible puzzle\n";
				return;
			}
		next_stack:
			//resort by the least# of choices left to increase speed
			++index;
			for (size_t idx = index; idx < selections.size(); idx++) {
				uint8_t count = 0;
				Selection pick = selections[idx];
				uint8_t* y_pick = &selection_cache[(0 * cache_size) + (pick.row * (cache_width))];
				uint8_t* x_pick = &selection_cache[(1 * cache_size) + (pick.col * (cache_width))];
				uint8_t* b_pick = &selection_cache[(2 * cache_size) + (pick.blk * (cache_width))];
				for (uint32_t digit = 1; digit < cache_width; digit++) {
					count += (y_pick[digit] || x_pick[digit] || b_pick[digit]);
				}
				uint8_t* c_pick = &selection_cache[(3 * cache_size) + (pick.row * board_len) + pick.col];
				*c_pick = count;
			}
			auto it = std::min_element(selections.begin() + index, selections.end(), [&selection_cache, cache_size, board_len](Selection& lhs, Selection& rhs) {
				size_t lhs_marks = 0;
				size_t rhs_marks = 0;

				lhs_marks = selection_cache[(3 * cache_size) + ((uint32_t)lhs.row * board_len) + lhs.col];
				rhs_marks = selection_cache[(3 * cache_size) + ((uint32_t)rhs.row * board_len) + rhs.col];

				return rhs_marks < lhs_marks;
				});
			if (index >= selections.size()) {
				break;
			}
			std::swap(selections[index], *it);
		}
		//write out to puzzle
		for (size_t i = 0; i < selections.size(); i++) {
			Selection pick = selections[i];
			board[pick.row][pick.col] = (uint8_t)pick.value + (uint8_t)'0';
		}
	}

	template<size_t N>
	void solveSudoku(std::array<std::array<char, N>, N>& board) {
		//ignore weird boards
		if (!board.size())
			return;
		if (board[0].size() != board.size())
			return;
		constexpr uint32_t board_len = N;
		constexpr uint32_t blk_len = patchwork::sqrt(N);
		if ((blk_len * blk_len) != board_len)
			return;

		constexpr uint32_t board_size = board_len * board_len;;
		std::array<Selection, board_size> selections{};

		constexpr uint32_t cache_width = board_len + 1;
		constexpr uint32_t cache_size = (board_len * cache_width);

		std::array<uint8_t, 3 * (board_len * cache_width)> selection_cache;
		std::array<uint8_t, board_size> cell_count;

		for (size_t i = 0; i < selection_cache.size(); i++)
			selection_cache[i] = 0;

		uint32_t known_locations = 0;
		{
			size_t tmp_idx = 0;
			size_t back_idx = board_size - 1;
			for (size_t y = 0; y < board.size(); y++) {
				for (size_t x = 0; x < board.size(); x++) {
					uint8_t digit = (uint8_t)board[y][x] - (uint8_t)'0';
					//char_to_digit[board[y][x]];
					if (digit <= board.size()) {
						known_locations++;
						uint8_t b = calculate_block(x, y, blk_len);

						uint8_t* y_cached = &selection_cache[(0 * cache_size) + (y * (cache_width)) + digit];
						uint8_t* x_cached = &selection_cache[(1 * cache_size) + (x * (cache_width)) + digit];
						uint8_t* b_cached = &selection_cache[(2 * cache_size) + (b * (cache_width)) + digit];
						if (*y_cached || *x_cached || *b_cached) {
							//bad start to the puzzle (impossible to solve b/c invalid)
							//std::cout << "already marked\n";
							return;
						}
						//selections.emplace_back((uint8_t)digit, (uint8_t)y, (uint8_t)x, (uint8_t)b);
						selections[tmp_idx].value = digit;
						selections[tmp_idx].row = y;
						selections[tmp_idx].col = x;
						selections[tmp_idx].blk = b;
						tmp_idx++;
						*y_cached = 1;
						*x_cached = 1;
						*b_cached = 1;
					}
					else {
						//fill in w/ 0 for unselected (from the back)
						uint8_t b = calculate_block(x, y, blk_len);
						//selections.emplace_back((uint8_t)0, (uint8_t)y, (uint8_t)x, (uint8_t)b);
						selections[back_idx].value = 0;
						selections[back_idx].row = y;
						selections[back_idx].col = x;
						selections[back_idx].blk = b;
						back_idx--;
					}
				}
			}
		}

		for (size_t idx = known_locations; idx < selections.size(); idx++) {
			uint8_t count = 0;
			Selection pick = selections[idx];
			uint8_t* y_pick = &selection_cache[(0 * cache_size) + (pick.row * (cache_width))];
			uint8_t* x_pick = &selection_cache[(1 * cache_size) + (pick.col * (cache_width))];
			uint8_t* b_pick = &selection_cache[(2 * cache_size) + (pick.blk * (cache_width))];
			for (uint32_t digit = 1; digit < cache_width; digit++) {
				count += (y_pick[digit] || x_pick[digit] || b_pick[digit]);
			}
			cell_count[(pick.row * board_len) + pick.col] = count;
		}
		auto it = std::min_element(selections.begin() + known_locations, selections.end(), [&cell_count, board_len](Selection& lhs, Selection& rhs) {
			return cell_count[(rhs.row * board_len) + rhs.col] < cell_count[(lhs.row * board_len) + lhs.col];
			});
		if (known_locations < selections.size()) {
			std::swap(selections[known_locations], *it);
		}

		size_t index = known_locations;
		while (index >= known_locations && index < selections.size()) {
			Selection& pick = selections[index];

			for (size_t digit = pick.value; ++digit <= board_len;) {
				uint8_t* y_cached = &selection_cache[(0 * cache_size) + ((uint32_t)pick.row * (cache_width)) + digit];
				uint8_t* x_cached = &selection_cache[(1 * cache_size) + ((uint32_t)pick.col * (cache_width)) + digit];
				uint8_t* b_cached = &selection_cache[(2 * cache_size) + ((uint32_t)pick.blk * (cache_width)) + digit];

				if (*y_cached || *x_cached || *b_cached) {
					//skip things we cannot use be b/c of other selections
					continue;
				}

				//remark cell with new selection
				uint8_t* y_prev = &selection_cache[(0 * cache_size) + ((uint32_t)pick.row * (cache_width)) + pick.value];
				uint8_t* x_prev = &selection_cache[(1 * cache_size) + ((uint32_t)pick.col * (cache_width)) + pick.value];
				uint8_t* b_prev = &selection_cache[(2 * cache_size) + ((uint32_t)pick.blk * (cache_width)) + pick.value];
				*y_prev = 0;
				*x_prev = 0;
				*b_prev = 0;
				*y_cached = 1;
				*x_cached = 1;
				*b_cached = 1;
				pick.value = digit;

				//break;
				goto next_stack;
			}
			// failed to fill particular cell
			--index;
			if (index >= known_locations && index < selections.size()) {
				// unmark cell
				uint8_t* y_prev = &selection_cache[(0 * cache_size) + ((uint32_t)pick.row * (cache_width)) + pick.value];
				uint8_t* x_prev = &selection_cache[(1 * cache_size) + ((uint32_t)pick.col * (cache_width)) + pick.value];
				uint8_t* b_prev = &selection_cache[(2 * cache_size) + ((uint32_t)pick.blk * (cache_width)) + pick.value];
				*y_prev = 0;
				*x_prev = 0;
				*b_prev = 0;
				pick.value = 0;
				continue;
			}
			else {
				// we walked back and undid the entire puzzle (impossible starting condition)
				//std::cout << "impossible puzzle\n";
				return;
			}
		next_stack:
			//resort by the least# of choices left to increase speed
			++index;
			//do the dirty work of counting again
			for (size_t idx = index; idx < selections.size(); idx++) {
				uint8_t count = 0;
				Selection pick = selections[idx];
				uint8_t* y_pick = &selection_cache[(0 * cache_size) + (pick.row * (cache_width))];
				uint8_t* x_pick = &selection_cache[(1 * cache_size) + (pick.col * (cache_width))];
				uint8_t* b_pick = &selection_cache[(2 * cache_size) + (pick.blk * (cache_width))];
				for (uint32_t digit = 1; digit < cache_width; digit++) {
					count += (y_pick[digit] || x_pick[digit] || b_pick[digit]);
				}
				cell_count[(pick.row * board_len) + pick.col] = count;
			}
			auto it = std::min_element(selections.begin() + index, selections.end(), [&cell_count, board_len](Selection& lhs, Selection& rhs) {
				return cell_count[(rhs.row * board_len) + rhs.col] < cell_count[(lhs.row * board_len) + lhs.col];
				});
			if (index >= selections.size()) {
				break;
			}
			std::swap(selections[index], *it);
		}

		//write out to puzzle
		for (size_t i = 0; i < selections.size(); i++) {
			Selection pick = selections[i];
			board[pick.row][pick.col] = (uint8_t)pick.value + (uint8_t)'0';
		}
	}

	template<size_t N>
	void solveSudoku2(std::array<std::array<char, N>, N>& board) {
		//ignore weird boards
		constexpr uint32_t board_len = N;
		if constexpr (!board_len)
			return;
		constexpr uint32_t blk_len = patchwork::sqrt(N);
		if constexpr ((blk_len * blk_len) != board_len)
			return;
		[[maybe_unreachable]];

		constexpr uint32_t board_size = board_len * board_len;;
		std::array<Selection, board_size> selections{};

		constexpr uint32_t cache_width = board_len + 1;
		constexpr uint32_t cache_size = (board_len * cache_width);

		std::array<uint8_t, 3 * (board_len * cache_width)> selection_cache;
		std::array<uint8_t, board_size> cell_count;
		std::array<uint8_t, board_size* cache_width> cell_mask;
		std::array<uint8_t, board_size* cache_width> invalid_mask;
		//std::array<uint8_t, board_size * cache_width> init_vals;

		for (size_t i = 0; i < selection_cache.size(); i++)
			selection_cache[i] = 0;

		for (size_t i = 0; i < cell_mask.size(); i++)
			cell_mask[i] = 0;

		for (size_t i = 0; i < invalid_mask.size(); i++)
			invalid_mask[i] = 0;

		//for (size_t i = 0; i < init_vals.size(); i++)
		//	init_vals[i] = 0;

		uint32_t known_locations = 0;
		{
			size_t tmp_idx = 0;
			size_t back_idx = board_size - 1;
			for (size_t y = 0; y < board.size(); y++) {
				for (size_t x = 0; x < board.size(); x++) {
					uint8_t digit = (uint8_t)board[y][x] - (uint8_t)'0';
					//char_to_digit[board[y][x]];
					if (digit <= board.size()) {
						known_locations++;
						uint8_t b = calculate_block(x, y, blk_len);

						uint8_t* y_cached = &selection_cache[(0 * cache_size) + (y * (cache_width)) + digit];
						uint8_t* x_cached = &selection_cache[(1 * cache_size) + (x * (cache_width)) + digit];
						uint8_t* b_cached = &selection_cache[(2 * cache_size) + (b * (cache_width)) + digit];
						if (*y_cached || *x_cached || *b_cached) {
							//bad start to the puzzle (impossible to solve b/c invalid)
							//std::cout << "already marked\n";
							return;
						}
						//selections.emplace_back((uint8_t)digit, (uint8_t)y, (uint8_t)x, (uint8_t)b);
						selections[tmp_idx].value = digit;
						selections[tmp_idx].row = y;
						selections[tmp_idx].col = x;
						selections[tmp_idx].blk = b;

						cell_mask[(digit * board_size) + (y * board_len) + x] = 1;
						cell_mask[(y * board_len) + x] = 1;
						for (uint32_t digit = 1; digit < cache_width; digit++)
							invalid_mask[(((y * board_len) + x) * cache_width) + digit] = 1;

						tmp_idx++;
						*y_cached = 1;
						*x_cached = 1;
						*b_cached = 1;
					}
					else {
						//fill in w/ 0 for unselected (from the back)
						uint8_t b = calculate_block(x, y, blk_len);
						//selections.emplace_back((uint8_t)0, (uint8_t)y, (uint8_t)x, (uint8_t)b);
						selections[back_idx].value = 0;
						selections[back_idx].row = y;
						selections[back_idx].col = x;
						selections[back_idx].blk = b;
						back_idx--;
					}
				}
			}
		}

		for (size_t idx = known_locations; idx < selections.size(); idx++) {
			uint8_t count = 0;
			Selection pick = selections[idx];
			uint8_t* y_pick = &selection_cache[(0 * cache_size) + (pick.row * (cache_width))];
			uint8_t* x_pick = &selection_cache[(1 * cache_size) + (pick.col * (cache_width))];
			uint8_t* b_pick = &selection_cache[(2 * cache_size) + (pick.blk * (cache_width))];

			uint32_t blk_y = (pick.row / blk_len) * blk_len;
			uint32_t blk_x = (pick.col / blk_len) * blk_len;

			uint8_t* s_pick = &invalid_mask[((pick.row * board_len) + pick.col) * cache_width];
			uint8_t* m_mask = &cell_mask[(pick.row * board_len) + pick.col];

			//uint8_t* i_info = &init_vals[((pick.row * board_len) + pick.col) * cache_width];
			//i_info[0] = 0;

			for (uint32_t digit = 1; digit < cache_width; digit++) {
				bool is_marked = (y_pick[digit] || x_pick[digit] || b_pick[digit]);

				count += is_marked;
				s_pick[digit] = (uint8_t)is_marked; //cell_mask at this point is 0
			}
			cell_count[(pick.row * board_len) + pick.col] = count;
		}

		auto it = std::min_element(selections.begin() + known_locations, selections.end(), [&cell_count, board_len](Selection& lhs, Selection& rhs) {
			return cell_count[(rhs.row * board_len) + rhs.col] < cell_count[(lhs.row * board_len) + lhs.col];
			});
		if (known_locations < selections.size()) {
			std::swap(selections[known_locations], *it);
		}

		size_t index = known_locations;
		while (index >= known_locations && index < selections.size()) {
			Selection& pick = selections[index];

			uint8_t* y_pick = &selection_cache[(0 * cache_size) + (pick.row * (cache_width))];
			uint8_t* x_pick = &selection_cache[(1 * cache_size) + (pick.col * (cache_width))];
			uint8_t* b_pick = &selection_cache[(2 * cache_size) + (pick.blk * (cache_width))];
			uint8_t* s_mask = &invalid_mask[(((pick.row * board_len) + pick.col) * cache_width)];

			for (size_t digit = pick.value; ++digit <= board_len;) {
				if (s_mask[digit]) {
					continue;
				}
				y_pick[pick.value] = 0;
				x_pick[pick.value] = 0;
				b_pick[pick.value] = 0;
				y_pick[digit] = 1;
				x_pick[digit] = 1;
				b_pick[digit] = 1;

				cell_mask[(pick.value * board_size) + (pick.row * board_len) + pick.col] = 0;

				cell_mask[(digit * board_size) + (pick.row * board_len) + pick.col] = 1;
				cell_mask[(pick.row * board_len) + pick.col] = 1;
				pick.value = digit;
				//pick.idx++;

				goto next_stack;
			}
		fail_stack:
			// failed to fill particular cell
			--index;
			if (index >= known_locations && index < selections.size()) {
				// unmark cell
				y_pick[pick.value] = 0;
				x_pick[pick.value] = 0;
				b_pick[pick.value] = 0;
				//pick.idx = 0;
				cell_mask[(pick.value * board_size) + (pick.row * board_len) + pick.col] = 0;
				cell_mask[(pick.row * board_len) + pick.col] = 0;
				pick.value = 0;
				continue;
			}
			else {
				// we walked back and undid the entire puzzle (impossible starting condition)
				//std::cout << "impossible puzzle\n";
				return;
			}
		next_stack:
			//prevent backtracking if we can show we have no choices left
			known_locations += (index == known_locations) && ((cell_count[(pick.row * board_len) + pick.col] == (board_len - 1)) || pick.value == board_len);
			++index;
			/*
			for (size_t i = 0; i < board_size; i++) {
				cell_count[i] = 0;
			}
			for (size_t digit = 1; digit < cache_width; digit++) {
				for (size_t idx = index; idx < selections.size(); idx++) {
					Selection pick = selections[idx];
					uint8_t* y_pick = &selection_cache[(0 * cache_size) + (pick.row * (cache_width))];
					uint8_t* x_pick = &selection_cache[(1 * cache_size) + (pick.col * (cache_width))];
					uint8_t* b_pick = &selection_cache[(2 * cache_size) + (pick.blk * (cache_width))];
					//uint8_t* s_pick = &invalid_mask[idx * cache_width];
					uint8_t* s_pick = &invalid_mask[((pick.row * board_len) + pick.col) * cache_width];

					bool is_marked = (y_pick[digit] || x_pick[digit] || b_pick[digit]);
					cell_count[(pick.row * board_len) + pick.col] += is_marked;
					s_pick[digit] = is_marked;
				}
			}
			*/
			
			for (size_t idx = index; idx < selections.size(); idx++) {
				uint8_t count = 0;
				Selection pick = selections[idx];
				uint8_t* y_pick = &selection_cache[(0 * cache_size) + (pick.row * (cache_width))];
				uint8_t* x_pick = &selection_cache[(1 * cache_size) + (pick.col * (cache_width))];
				uint8_t* b_pick = &selection_cache[(2 * cache_size) + (pick.blk * (cache_width))];

				uint32_t blk_y = (pick.row / blk_len) * blk_len;
				uint32_t blk_x = (pick.col / blk_len) * blk_len;

				uint8_t* s_pick = &invalid_mask[((pick.row * board_len) + pick.col) * cache_width];
				for (uint32_t digit = 1; digit < cache_width; digit++) {
					bool is_marked = (y_pick[digit] || x_pick[digit] || b_pick[digit]);
					count += is_marked;
					s_pick[digit] = is_marked;
				}
				cell_count[(pick.row * board_len) + pick.col] = count;
			}
			
			auto it = std::min_element(selections.begin() + index, selections.end(), [&cell_count, board_len](Selection& lhs, Selection& rhs) {
				return cell_count[(rhs.row * board_len) + rhs.col] < cell_count[(lhs.row * board_len) + lhs.col];
				});
			if (index >= selections.size()) {
				break;
			}
			std::swap(selections[index], *it);
		}

		//write out to puzzle
		for (size_t i = 0; i < selections.size(); i++) {
			Selection pick = selections[i];
			board[pick.row][pick.col] = (uint8_t)pick.value + (uint8_t)'0';
		}
	}

	template<size_t N>
	void solveSudoku4(std::array<std::array<char, N>, N>& board) {
		//ignore weird boards
		constexpr uint32_t board_len = N;
		if constexpr (!board_len)
			return;
		constexpr uint32_t blk_len = patchwork::sqrt(N);
		if constexpr ((blk_len * blk_len) != board_len)
			return;
		[[maybe_unreachable]];

		constexpr uint32_t board_size = board_len * board_len;;
		std::array<Selection, board_size> selections{};

		constexpr uint32_t cache_width = board_len + 1;
		constexpr uint32_t cache_size = (board_len * cache_width);

		std::array<std::bitset<cache_width>, 3 * board_len> bitwise_cache;
		std::array<std::bitset<cache_width>, board_size> bitwise_mask;

		std::array<uint8_t, board_size> cell_count;

		for (size_t i = 0; i < bitwise_cache.size(); i++)
			bitwise_cache[i] = 0;
		for (size_t i = 0; i < bitwise_mask.size(); i++)
			bitwise_mask[i] = 0;

		uint32_t known_locations = 0;
		{
			size_t tmp_idx = 0;
			size_t back_idx = board_size - 1;
			for (size_t y = 0; y < board.size(); y++) {
				for (size_t x = 0; x < board.size(); x++) {
					uint8_t digit = (uint8_t)board[y][x] - (uint8_t)'0';
					//char_to_digit[board[y][x]];
					if (digit <= board.size()) {
						known_locations++;
						uint8_t b = calculate_block(x, y, blk_len);

						std::bitset<cache_width> y_cached = bitwise_cache[(0 * board_len) + y];
						std::bitset<cache_width> x_cached = bitwise_cache[(1 * board_len) + x];
						std::bitset<cache_width> b_cached = bitwise_cache[(2 * board_len) + b];
						if (y_cached[digit] | x_cached[digit] | b_cached[digit]) {
							return;
						}
						selections[tmp_idx].value = digit;
						selections[tmp_idx].row = y;
						selections[tmp_idx].col = x;
						selections[tmp_idx].blk = b;

						tmp_idx++;

						bitwise_cache[(0 * board_len) + y][digit] = 1;
						bitwise_cache[(1 * board_len) + x][digit] = 1;
						bitwise_cache[(2 * board_len) + b][digit] = 1;

					}
					else {
						//fill in w/ 0 for unselected (from the back)
						uint8_t b = calculate_block(x, y, blk_len);
						selections[back_idx].value = 0;
						selections[back_idx].row = y;
						selections[back_idx].col = x;
						selections[back_idx].blk = b;
						back_idx--;
					}
				}
			}
		}

		for (size_t idx = known_locations; idx < selections.size(); idx++) {
			Selection pick = selections[idx];
			//performs faster on msvc...
			std::bitset<cache_width> combined = bitwise_cache[(0 * board_len) + pick.row];
			combined |= bitwise_cache[(1 * board_len) + pick.col];
			combined |= bitwise_cache[(2 * board_len) + pick.blk];
			/*
			std::bitset<cache_width> y_cached = bitwise_cache[(0 * board_len) + pick.row];
			std::bitset<cache_width> x_cached = bitwise_cache[(1 * board_len) + pick.col];
			std::bitset<cache_width> b_cached = bitwise_cache[(2 * board_len) + pick.blk];
			*/

			//std::bitset<cache_width> combined = y_cached | x_cached | b_cached;
			bitwise_mask[((pick.row * board_len) + pick.col)] = combined;
			uint8_t count = combined.count();
			cell_count[(pick.row * board_len) + pick.col] = count;
		}

		auto it = std::min_element(selections.begin() + known_locations, selections.end(), [&cell_count, board_len](Selection& lhs, Selection& rhs) {
			return cell_count[(rhs.row * board_len) + rhs.col] < cell_count[(lhs.row * board_len) + lhs.col];
			});
		if (known_locations < selections.size()) {
			std::swap(selections[known_locations], *it);
		}

		size_t index = known_locations;
		while (index >= known_locations && index < selections.size()) {
			Selection& pick = selections[index];
			std::bitset<cache_width> s_mask = bitwise_mask[((pick.row * board_len) + pick.col)];
			for (size_t digit = pick.value; ++digit <= board_len;) {
				if (s_mask[digit]) {
					continue;
				}
				/*
				0 1
				1 0
				*/
				/*
				std::bitset<cache_width> tmp;
				tmp.set(pick.value, true);
				tmp.set(digit, true);
				bitwise_cache[(0 * board_len) + pick.row] ^= tmp;
				bitwise_cache[(1 * board_len) + pick.col] ^= tmp;
				bitwise_cache[(2 * board_len) + pick.blk] ^= tmp;
				*/
				/*
				bitwise_cache[(0 * board_len) + pick.row].flip(pick.value).flip(digit);
				bitwise_cache[(1 * board_len) + pick.col].flip(pick.value).flip(digit);
				bitwise_cache[(2 * board_len) + pick.blk].flip(pick.value).flip(digit);
				*/
				bitwise_cache[(0 * board_len) + pick.row][pick.value] = 0;
				bitwise_cache[(1 * board_len) + pick.col][pick.value] = 0;
				bitwise_cache[(2 * board_len) + pick.blk][pick.value] = 0;

				bitwise_cache[(0 * board_len) + pick.row][digit] = 1;
				bitwise_cache[(1 * board_len) + pick.col][digit] = 1;
				bitwise_cache[(2 * board_len) + pick.blk][digit] = 1;
				pick.value = digit;

				goto next_stack;
			}
		fail_stack:
			// failed to fill particular cell
			--index;
			if (index >= known_locations && index < selections.size()) {
				// unmark cell (backtrack)
				bitwise_cache[(0 * board_len) + pick.row][pick.value] = 0;
				bitwise_cache[(1 * board_len) + pick.col][pick.value] = 0;
				bitwise_cache[(2 * board_len) + pick.blk][pick.value] = 0;
				pick.value = 0;
				continue;
			}
			else {
				// we walked back and undid the entire puzzle (impossible starting condition)
				//std::cout << "impossible puzzle\n";
				return;
			}
		next_stack:
			//prevent backtracking if we can show we have no choices left
			known_locations += (index == known_locations) && ((cell_count[(pick.row * board_len) + pick.col] == (board_len - 1)) || pick.value == board_len);
			//cell_count[(pick.row * board_len) + pick.col]--;
			//resort by the least# of choices left to increase speed
			++index;
			//do the dirty work of counting again
			size_t mx_index = index;
			size_t prev_count = 0;
			for (size_t idx = index; idx < selections.size(); idx++) {
				Selection pick = selections[idx];

				std::bitset<cache_width> combined = bitwise_cache[(0 * board_len) + pick.row];
				combined |= bitwise_cache[(1 * board_len) + pick.col];
				combined |= bitwise_cache[(2 * board_len) + pick.blk];

				bitwise_mask[((pick.row * board_len) + pick.col)] = combined;
				if constexpr (N < 64) {
					size_t count = _mm_popcnt_u64(combined.to_ullong());
					//size_t count = combined.count();
					cell_count[(pick.row * board_len) + pick.col] = count;
					if (count > prev_count) {
						prev_count = count;
						mx_index = idx;
					}
				}
				else {
					//size_t count = _mm_popcnt_u64(combined.to_ullong());
					size_t count = combined.count();
					cell_count[(pick.row * board_len) + pick.col] = count;
					if (count > prev_count) {
						prev_count = count;
						mx_index = idx;
					}
				}
			}
			/*
			auto it = std::min_element(selections.begin() + index, selections.end(), [&cell_count, board_len](Selection& lhs, Selection& rhs) {
				return cell_count[(rhs.row * board_len) + rhs.col] < cell_count[(lhs.row * board_len) + lhs.col];
				});
				*/
			if (index >= selections.size()) {
				break;
			}
			std::swap(selections[index], selections[mx_index]);
			//std::swap(selections[index], *it);
		}

		//write out to puzzle
		for (size_t i = 0; i < selections.size(); i++) {
			Selection pick = selections[i];
			board[pick.row][pick.col] = (uint8_t)pick.value + (uint8_t)'0';
		}
	}

	template<size_t N>
	void solveSudoku5(std::array<std::array<char, N>, N>& board) {
		//ignore weird boards
		constexpr uint32_t board_len = N;
		if constexpr (!board_len)
			return;
		constexpr uint32_t blk_len = patchwork::sqrt(N);
		if constexpr ((blk_len * blk_len) != board_len)
			return;
		[[maybe_unreachable]];

		constexpr uint32_t board_size = board_len * board_len;;
		std::array<Selection, board_size> selections{};

		constexpr uint32_t cache_width = board_len + 1;
		constexpr uint32_t cache_size = (board_len * cache_width);

		std::array<std::bitset<cache_width>, 3 * board_len> bitwise_cache;
		std::array<std::bitset<cache_width>, board_size> bitwise_mask;

		std::array<uint8_t, board_size> cell_count;

		for (size_t i = 0; i < bitwise_cache.size(); i++)
			bitwise_cache[i] = 0;
		for (size_t i = 0; i < bitwise_mask.size(); i++)
			bitwise_mask[i] = 0;

		uint32_t known_locations = 0;
		{
			size_t tmp_idx = 0;
			size_t back_idx = board_size - 1;
			for (size_t y = 0; y < board.size(); y++) {
				for (size_t x = 0; x < board.size(); x++) {
					uint8_t digit = (uint8_t)board[y][x] - (uint8_t)'0';
					//char_to_digit[board[y][x]];
					if (digit <= board.size()) {
						known_locations++;
						uint8_t b = calculate_block(x, y, blk_len);

						std::bitset<cache_width> y_cached = bitwise_cache[(0 * board_len) + y];
						std::bitset<cache_width> x_cached = bitwise_cache[(1 * board_len) + x];
						std::bitset<cache_width> b_cached = bitwise_cache[(2 * board_len) + b];
						if (y_cached[digit] | x_cached[digit] | b_cached[digit]) {
							return;
						}
						selections[tmp_idx].value = digit;
						selections[tmp_idx].row = y;
						selections[tmp_idx].col = x;
						selections[tmp_idx].blk = b;
						/*
						cell_mask[(digit * board_size) + (y * board_len) + x] = 1;
						cell_mask[(y * board_len) + x] = 1;
						for (uint32_t digit = 1; digit < cache_width; digit++)
							invalid_mask[(((y * board_len) + x) * cache_width) + digit] = 1;
							*/
						tmp_idx++;

						bitwise_cache[(0 * board_len) + y][digit] = 1;
						bitwise_cache[(1 * board_len) + x][digit] = 1;
						bitwise_cache[(2 * board_len) + b][digit] = 1;

					}
					else {
						//fill in w/ 0 for unselected (from the back)
						uint8_t b = calculate_block(x, y, blk_len);
						selections[back_idx].value = 0;
						selections[back_idx].row = y;
						selections[back_idx].col = x;
						selections[back_idx].blk = b;
						back_idx--;
					}
				}
			}
		}

		for (size_t idx = known_locations; idx < selections.size(); idx++) {
			Selection pick = selections[idx];
			//performs faster on msvc...
			std::bitset<cache_width> combined = bitwise_cache[(0 * board_len) + pick.row];
			combined |= bitwise_cache[(1 * board_len) + pick.col];
			combined |= bitwise_cache[(2 * board_len) + pick.blk];

			std::bitset<cache_width> combined = y_cached | x_cached | b_cached;
			bitwise_mask[((pick.row * board_len) + pick.col)] = combined;
			uint8_t count = combined.count();
			cell_count[(pick.row * board_len) + pick.col] = count;
		}

		auto it = std::min_element(selections.begin() + known_locations, selections.end(), [&cell_count, board_len](Selection& lhs, Selection& rhs) {
			return cell_count[(rhs.row * board_len) + rhs.col] < cell_count[(lhs.row * board_len) + lhs.col];
			});
		if (known_locations < selections.size()) {
			std::swap(selections[known_locations], *it);
		}

		size_t index = known_locations;
		while (index >= known_locations && index < selections.size()) {
			Selection& pick = selections[index];
			std::bitset<cache_width> s_mask = bitwise_mask[((pick.row * board_len) + pick.col)];
			for (size_t digit = pick.value; ++digit <= board_len;) {
				if (s_mask[digit]) {
					continue;
				}
				bitwise_cache[(0 * board_len) + pick.row][pick.value] = 0;
				bitwise_cache[(1 * board_len) + pick.col][pick.value] = 0;
				bitwise_cache[(2 * board_len) + pick.blk][pick.value] = 0;

				bitwise_cache[(0 * board_len) + pick.row][digit] = 1;
				bitwise_cache[(1 * board_len) + pick.col][digit] = 1;
				bitwise_cache[(2 * board_len) + pick.blk][digit] = 1;
				pick.value = digit;

				goto next_stack;
			}
		fail_stack:
			// failed to fill particular cell
			--index;
			if (index >= known_locations && index < selections.size()) {
				// unmark cell (backtrack)
				bitwise_cache[(0 * board_len) + pick.row][pick.value] = 0;
				bitwise_cache[(1 * board_len) + pick.col][pick.value] = 0;
				bitwise_cache[(2 * board_len) + pick.blk][pick.value] = 0;
				pick.value = 0;
				continue;
			}
			else {
				// we walked back and undid the entire puzzle (impossible starting condition)
				//std::cout << "impossible puzzle\n";
				return;
			}
		next_stack:
			//prevent backtracking if we can show we have no choices left
			known_locations += (index == known_locations) && ((cell_count[(pick.row * board_len) + pick.col] == (board_len - 1)) || pick.value == board_len);
			//cell_count[(pick.row * board_len) + pick.col]--;
			//resort by the least# of choices left to increase speed
			++index;
			//do the dirty work of counting again
			size_t mx_index = index;
			size_t prev_count = 0;
			for (size_t idx = index; idx < selections.size(); idx++) {
				Selection pick = selections[idx];

				std::bitset<cache_width> combined = bitwise_cache[(0 * board_len) + pick.row];
				combined |= bitwise_cache[(1 * board_len) + pick.col];
				combined |= bitwise_cache[(2 * board_len) + pick.blk];

				bitwise_mask[((pick.row * board_len) + pick.col)] = combined;
				if constexpr (N < 64) {
					size_t count = _mm_popcnt_u64(combined.to_ullong());
					//size_t count = combined.count();
					cell_count[(pick.row * board_len) + pick.col] = count;
					if (count > prev_count) {
						prev_count = count;
						mx_index = idx;
					}
				}
				else {
					//size_t count = _mm_popcnt_u64(combined.to_ullong());
					size_t count = combined.count();
					cell_count[(pick.row * board_len) + pick.col] = count;
					if (count > prev_count) {
						prev_count = count;
						mx_index = idx;
					}
				}
			}

			if (index >= selections.size()) {
				break;
			}
			std::swap(selections[index], selections[mx_index]);
			//std::swap(selections[index], *it);
		}

		//write out to puzzle
		for (size_t i = 0; i < selections.size(); i++) {
			Selection pick = selections[i];
			board[pick.row][pick.col] = (uint8_t)pick.value + (uint8_t)'0';
		}
	}

	static constexpr uint32_t _cache_width = 256;
	std::vector<SelectionInfo> _selections;
	std::vector<std::bitset<_cache_width>> _bit_masks;
	std::vector<uint8_t> _cell_count;

	bool solveSudoku(const char* input, size_t limit, uint32_t configuration, char* solution, size_t* num_guesses) {
		//ignore weird boards

		*num_guesses = 0;

		if (!limit)
			return false;
		const size_t board_size = limit;
		const size_t board_len = patchwork::sqrt(limit);
		if ((board_len * board_len) != board_size)
			return false;
		const size_t blk_len = patchwork::sqrt(board_len);
		if ((blk_len * blk_len) != board_len)
			return false;
		//too large for this algorithm
		if (board_len > 254)
			return false;

		_selections.clear();
		_selections.reserve(board_size + 1);
		for (size_t i = 0; i < (board_size + 1); i++) {
			_selections.emplace_back();
		}
		//(2*(board_len * (board_len+1))

		uint32_t bitmask_size = (3 * board_len) + board_size + ((board_len + 1) * (board_len + 1));
		_bit_masks.clear();
		_bit_masks.reserve(bitmask_size);
		for (size_t i = 0; i < bitmask_size; i++) {
			_bit_masks.emplace_back(0);
		}

		uint32_t cell_count_size = board_size + ((board_len + 1) * (board_len + 1));
		_cell_count.clear();
		_cell_count.reserve(cell_count_size);
		for (size_t i = 0; i < cell_count_size; i++) {
			_cell_count.emplace_back(0);
		}

		size_t known_locations = 0;
		size_t back_idx = board_size - 1;
		for (size_t xy = 0; xy < board_size; xy++) {
			uint32_t y = xy / board_len;
			uint32_t x = xy % board_len;
			uint32_t b = calculate_block(x, y, blk_len);
			uint32_t bi = calculate_block_idx(x, y, blk_len);
			uint8_t digit = ((uint8_t)input[xy] - (uint8_t)'0');
			if (digit <= board_len) {
				SelectionInfo& item = _selections[known_locations];
				//cartesean coordinate
				item.col = x;
				item.row = y;
				//block coordinate
				item.blk = b;
				item.blk_idx = bi;
				item.value = digit;

				std::bitset<_cache_width>& y_cached = _bit_masks[(0 * board_len) + y];
				std::bitset<_cache_width>& x_cached = _bit_masks[(1 * board_len) + x];
				std::bitset<_cache_width>& b_cached = _bit_masks[(2 * board_len) + b];
				if (y_cached[digit] | x_cached[digit] | b_cached[digit]) {
					return false;
				}

				std::bitset<_cache_width>& cell_digit_mask = _bit_masks[((3 * board_len) + board_size) +
					(digit * (board_len + 1)) + b
				];
				cell_digit_mask[bi] = 1;

				std::bitset<_cache_width>& cell_mask = _bit_masks[((3 * board_len) + board_size) +
					(0 * (board_len + 1)) + b
				];
				cell_mask[bi] = 1;

				y_cached[digit] = 1;
				x_cached[digit] = 1;
				b_cached[digit] = 1;

				known_locations++;
			}
			else {
				SelectionInfo& item = _selections[back_idx];
				item.col = x;
				item.row = y;
				item.blk = b;
				item.blk_idx = bi;
				item.value = 0;
				back_idx--;
			}
		}

		{
			size_t prev_count = 0;
			size_t mx_index = known_locations;

			uint32_t cell_idx = ((3 * board_len) + board_size);
			for (size_t idx = known_locations; idx < board_size; idx++) {
				SelectionInfo pick = _selections[idx];
				//performs faster on msvc...
				/*
				std::bitset<_cache_width> combined = _bit_masks[(0 * board_len) + pick.row];
				combined |= _bit_masks[(1 * board_len) + pick.col];
				combined |= _bit_masks[(2 * board_len) + pick.blk];

				_bit_masks[(3 * board_len) + ((pick.row * board_len) + pick.col)] = combined;
				*/
				std::bitset<_cache_width>& combined = _bit_masks[(3 * board_len) + ((pick.row * board_len) + pick.col)];
				combined = _bit_masks[(0 * board_len) + pick.row];
				combined |= _bit_masks[(1 * board_len) + pick.col];
				combined |= _bit_masks[(2 * board_len) + pick.blk];
				//_bit_masks[(0 * board_len) + pick.row];

				bool any_set = false;
				for (size_t digit = 1; digit < board_len; digit++) {
					_bit_masks[cell_idx + (digit * (board_len + 1)) + pick.blk][pick.blk_idx] = combined[digit];
				}
			}
			//combine all masks once
			uint32_t tmp_blk = 0;
			uint32_t tmp_limit = ((board_len+1) * (board_len + 1));
			for (size_t tmp = (1 * (board_len + 1)); tmp < tmp_limit; tmp++) {
				std::bitset<_cache_width>& blk_mask = _bit_masks[cell_idx + tmp];
				blk_mask |= _bit_masks[cell_idx + tmp_blk];
				tmp_blk = (tmp_blk + 1) % (board_len + 1);

				uint64_t* ptr = (uint64_t*)&blk_mask;
				uint64_t count = _mm_popcnt_u64(*ptr);
				count += _mm_popcnt_u64(*(ptr + 1));
				count += _mm_popcnt_u64(*(ptr + 2));
				count += _mm_popcnt_u64(*(ptr + 3));

				_cell_count[board_size + tmp] = count;
			}
			//now all the required masks are filled, we can now do two checks at once
			for (size_t idx = known_locations; idx < board_size; idx++) {
				SelectionInfo pick = _selections[idx];

				uint32_t cell_target_idx = ((3 * board_len) + board_size) + ((board_len) * (board_len + 1));

				uint32_t pick_blk_row = 0;
				uint32_t pick_blk_col = 0;
				//_bit_masks[cell_idx + (digit * (board_len + 1)) + pick.blk]
				std::bitset<_cache_width>& combined = _bit_masks[(3 * board_len) + ((pick.row * board_len) + pick.col)];

				//simple count, the possible values for this cell were masked earlier
				uint64_t* ptr = (uint64_t*)&combined;
				uint64_t count = _mm_popcnt_u64(*ptr);
				count += _mm_popcnt_u64(*(ptr + 1));
				count += _mm_popcnt_u64(*(ptr + 2));
				count += _mm_popcnt_u64(*(ptr + 3));

				//now we check spacing constraints
				for (size_t digit = 1; digit < board_len; digit++) {
					/*
					std::bitset<_cache_width> blk_mask = _bit_masks[cell_idx + (digit * (board_len + 1)) + pick.blk] |
						_bit_masks[cell_idx + (0 * (board_len + 1)) + pick.blk];
						*/
					/*
					std::bitset<_cache_width>& blk_mask = _bit_masks[cell_idx + (digit * (board_len + 1)) + pick.blk];
					blk_mask |= _bit_masks[cell_idx + (0 * (board_len + 1)) + pick.blk];
					*/
					uint32_t blk_flat_idx = (digit * (board_len + 1)) + pick.blk;
					std::bitset<_cache_width>& blk_mask = _bit_masks[cell_idx + blk_flat_idx];
					uint8_t current_count = _cell_count[board_size + blk_flat_idx];
					/*
					uint64_t* blk_ptr = (uint64_t*)&blk_mask;

					uint64_t current_count = _mm_popcnt_u64(*blk_ptr);
					current_count += _mm_popcnt_u64(*(blk_ptr + 1));
					current_count += _mm_popcnt_u64(*(blk_ptr + 2));
					current_count += _mm_popcnt_u64(*(blk_ptr + 3));
					*/
					/*
					size_t current_count = blk_mask.count();
					*/
					//space_count = (current_count > space_count) ? current_count : space_count;
					if (current_count == (board_len - 1) && blk_mask[pick.blk_idx] == 0) {
						//create a bitmask
						std::bitset<_cache_width> tmp;
						tmp.set();
						tmp.reset(digit);
						//there's only 1 spot for this value, mark all others 
						combined |= tmp;

						uint64_t n_count = _mm_popcnt_u64(*ptr);
						n_count += _mm_popcnt_u64(*(ptr + 1));
						n_count += _mm_popcnt_u64(*(ptr + 2));
						n_count += _mm_popcnt_u64(*(ptr + 3));
						//must be > than count no need to conditionally move
						count = n_count;
					}
				}

				_cell_count[(pick.row * board_len) + pick.col] = count;
				mx_index = (count > prev_count) ? idx : mx_index;
				prev_count = (count > prev_count) ? count : prev_count;
			}
			//we reserved +1 to do this unconditionally*
			std::swap(_selections[known_locations], _selections[mx_index]);
		}

		size_t index = known_locations;
		while (index >= known_locations && index < board_size) {
		start_stack:
			SelectionInfo& pick = _selections[index];
			std::bitset<_cache_width> s_mask = _bit_masks[(3 * board_len) + ((pick.row * board_len) + pick.col)];
			for (size_t digit = pick.value; ++digit <= board_len;) {
				if (s_mask[digit]) {
					continue;
				}
				(*num_guesses)++;

				std::bitset<_cache_width>& cell_value_mask = _bit_masks[((3 * board_len) + board_size) +
					(pick.value * (board_len + 1)) + pick.blk
				];
				cell_value_mask[pick.blk_idx] = 0;

				std::bitset<_cache_width>& cell_digit_mask = _bit_masks[((3 * board_len) + board_size) +
					(digit * (board_len + 1)) + pick.blk
				];
				cell_digit_mask[pick.blk_idx] = 1;

				std::bitset<_cache_width>& cell_mask = _bit_masks[((3 * board_len) + board_size) +
					(0 * (board_len + 1)) + pick.blk
				];
				cell_mask[pick.blk_idx] = 1;

				_bit_masks[(0 * board_len) + pick.row][pick.value] = 0;
				_bit_masks[(1 * board_len) + pick.col][pick.value] = 0;
				_bit_masks[(2 * board_len) + pick.blk][pick.value] = 0;

				_bit_masks[(0 * board_len) + pick.row][digit] = 1;
				_bit_masks[(1 * board_len) + pick.col][digit] = 1;
				_bit_masks[(2 * board_len) + pick.blk][digit] = 1;

				pick.value = digit;

				goto next_stack;
			}
			//we don't jump here
		fail_stack:
			// failed to fill particular cell
			--index;
			if (index >= known_locations && index < board_size) {
				std::bitset<_cache_width>& cell_mask = _bit_masks[((3 * board_len) + board_size) +
					(0 * (board_len + 1)) + pick.blk
				];
				cell_mask[pick.blk_idx] = 0;

				std::bitset<_cache_width>& cell_value_mask = _bit_masks[((3 * board_len) + board_size) +
					(pick.value * (board_len + 1)) + pick.blk
				];
				cell_value_mask[pick.blk_idx] = 0;

				// unmark cell (backtrack)
				_bit_masks[(0 * board_len) + pick.row][pick.value] = 0;
				_bit_masks[(1 * board_len) + pick.col][pick.value] = 0;
				_bit_masks[(2 * board_len) + pick.blk][pick.value] = 0;
				pick.value = 0;
				//we already did our conditional test in theory should get optimized away
				//goto start_stack;
				continue;
			}
			else {
				// we walked back and undid the entire puzzle (impossible starting condition)
				return false;
			}
		next_stack:
			//prevent backtracking if we can show we have no choices left
			known_locations += (index == known_locations) && ((_cell_count[(pick.row * board_len) + pick.col] == (board_len - 1)) || pick.value == board_len);
			//cell_count[(pick.row * board_len) + pick.col]--;
			//resort by the least# of choices left to increase speed
			++index;
			if (index >= board_size) {
				break;
			}

			//do the dirty work of counting again
			size_t prev_count = 0;
			size_t mx_index = index;

			const size_t row_index = 0 * board_len;
			const size_t col_index = 1 * board_len;
			const size_t blk_index = 2 * board_len;
			const size_t msk_index = 3 * board_len;

			{
				uint32_t cell_idx = ((3 * board_len) + board_size);
				for (size_t idx = index; idx < board_size; idx++) {
					SelectionInfo pick = _selections[idx];
					//performs faster on msvc...
					/*
					std::bitset<_cache_width> combined = _bit_masks[(0 * board_len) + pick.row];
					combined |= _bit_masks[(1 * board_len) + pick.col];
					combined |= _bit_masks[(2 * board_len) + pick.blk];

					_bit_masks[(3 * board_len) + ((pick.row * board_len) + pick.col)] = combined;
					*/
					std::bitset<_cache_width>& combined = _bit_masks[(3 * board_len) + ((pick.row * board_len) + pick.col)];
					combined = _bit_masks[(0 * board_len) + pick.row];
					combined |= _bit_masks[(1 * board_len) + pick.col];
					combined |= _bit_masks[(2 * board_len) + pick.blk];
					//_bit_masks[(0 * board_len) + pick.row];

					bool any_set = false;
					for (size_t digit = 1; digit < board_len; digit++) {
						_bit_masks[cell_idx + (digit * (board_len + 1)) + pick.blk][pick.blk_idx] = combined[digit];
					}
				}

				//combine all masks once
				uint32_t tmp_blk = 0;
				const uint32_t mask_len = board_len + 1;
				uint32_t tmp_limit = (mask_len * mask_len);

				for (size_t tmp = (1 * (board_len + 1)); tmp < tmp_limit; tmp++) {
					std::bitset<_cache_width>& blk_mask = _bit_masks[cell_idx + tmp];
					blk_mask |= _bit_masks[cell_idx + tmp_blk];
					tmp_blk = (tmp_blk + 1) % mask_len;

					uint64_t* ptr = (uint64_t*)&blk_mask;
					uint64_t count = _mm_popcnt_u64(*ptr);
					count += _mm_popcnt_u64(*(ptr + 1));
					count += _mm_popcnt_u64(*(ptr + 2));
					count += _mm_popcnt_u64(*(ptr + 3));

					_cell_count[board_size + tmp] = count;
				}

				//now all the required masks are filled, we can now do two checks at once
				for (size_t idx = index; idx < board_size; idx++) {
					SelectionInfo pick = _selections[idx];

					uint32_t cell_target_idx = cell_idx + ((board_len) * (board_len + 1));

					//_bit_masks[cell_idx + (digit * (board_len + 1)) + pick.blk]
					std::bitset<_cache_width>& combined = _bit_masks[(3 * board_len) + ((pick.row * board_len) + pick.col)];

					//simple count, the possible values for this cell were masked earlier
					uint64_t* ptr = (uint64_t*)&combined;
					uint64_t count = _mm_popcnt_u64(*ptr);
					count += _mm_popcnt_u64(*(ptr + 1));
					count += _mm_popcnt_u64(*(ptr + 2));
					count += _mm_popcnt_u64(*(ptr + 3));

					//now we check spacing constraints
					for (size_t digit = 1; digit < board_len; digit++) {
						//std::bitset<_cache_width>& blk_mask = _bit_masks[cell_idx + (digit * (board_len + 1)) + pick.blk];
							//| _bit_masks[cell_idx + (0 * (board_len + 1)) + pick.blk];

						uint32_t blk_flat_idx = (digit * (board_len + 1)) + pick.blk;
						std::bitset<_cache_width>& blk_mask = _bit_masks[cell_idx + blk_flat_idx];
						uint8_t current_count = _cell_count[board_size + blk_flat_idx];

						/*
						uint64_t* blk_ptr = (uint64_t*)&blk_mask;
						uint64_t current_count = _mm_popcnt_u64(*blk_ptr);
						current_count += _mm_popcnt_u64(*(blk_ptr + 1));
						current_count += _mm_popcnt_u64(*(blk_ptr + 2));
						current_count += _mm_popcnt_u64(*(blk_ptr + 3));
						*/
						/*
						size_t current_count = blk_mask.count();
						*/
						//space_count = (current_count > space_count) ? current_count : space_count;
						if (current_count == (board_len - 1) && blk_mask[pick.blk_idx] == 0) {
							//create a bitmask
							std::bitset<_cache_width> tmp;
							tmp.set();
							tmp.reset(digit);
							//there's only 1 spot for this value, mark all others 
							combined |= tmp;

							uint64_t n_count = _mm_popcnt_u64(*ptr);
							n_count += _mm_popcnt_u64(*(ptr + 1));
							n_count += _mm_popcnt_u64(*(ptr + 2));
							n_count += _mm_popcnt_u64(*(ptr + 3));
							count = n_count;
						}
						//_bit_masks[]
						//_bit_masks[cell_idx + (digit * (board_len + 1)) + pick.blk][pick.blk_idx] = combined[digit];
					}

					_cell_count[(pick.row * board_len) + pick.col] = count;
					mx_index = (count > prev_count) ? idx : mx_index;
					prev_count = (count > prev_count) ? count : prev_count;
				}

				//we reserved +1 to do this unconditionally*
				std::swap(_selections[index], _selections[mx_index]);
			}
			//std::swap(selections[index], *it);
		}

		//write out to puzzle
		for (size_t i = 0; i < board_size; i++) {
			SelectionInfo pick = _selections[i];
			solution[(pick.row * board_len) + pick.col] = ((uint8_t)pick.value + (uint8_t)'0');
			//board[pick.row][pick.col] = (uint8_t)pick.value + (uint8_t)'0';
		}
		return true;
	}
	bool solveSudokuSimple(const char* input, size_t limit, uint32_t configuration, char* solution, size_t* num_guesses) {
		//ignore weird boards

		*num_guesses = 0;

		if (!limit)
			return false;
		const size_t board_size = limit;
		const size_t board_len = patchwork::sqrt(limit);
		if ((board_len * board_len) != board_size)
			return false;
		const size_t blk_len = patchwork::sqrt(board_len);
		if ((blk_len * blk_len) != board_len)
			return false;
		//too large for this algorithm
		if (board_len > 254)
			return false;

		_selections.clear();
		_selections.reserve(board_size + 1);
		for (size_t i = 0; i < (board_size + 1); i++) {
			_selections.emplace_back();
		}
		//(2*(board_len * (board_len+1))

		uint32_t bitmask_size = (3 * board_len) + board_size + ((board_len + 1) * (board_len + 1));
		_bit_masks.clear();
		_bit_masks.reserve(bitmask_size);
		for (size_t i = 0; i < bitmask_size; i++) {
			_bit_masks.emplace_back(0);
		}

		uint32_t cell_count_size = board_size * 2;
		_cell_count.clear();
		_cell_count.reserve(cell_count_size);
		for (size_t i = 0; i < cell_count_size; i++) {
			_cell_count.emplace_back(0);
		}

		size_t known_locations = 0;
		size_t back_idx = board_size - 1;
		for (size_t xy = 0; xy < board_size; xy++) {
			uint32_t y = xy / board_len;
			uint32_t x = xy % board_len;
			uint32_t b = calculate_block(x, y, blk_len);
			uint32_t bi = calculate_block_idx(x, y, blk_len);
			uint8_t digit = ((uint8_t)input[xy] - (uint8_t)'0');
			if (digit <= board_len) {
				SelectionInfo& item = _selections[known_locations];
				//cartesean coordinate
				item.col = x;
				item.row = y;
				//block coordinate
				item.blk = b;
				item.blk_idx = bi;
				item.value = digit;

				std::bitset<_cache_width>& y_cached = _bit_masks[(0 * board_len) + y];
				std::bitset<_cache_width>& x_cached = _bit_masks[(1 * board_len) + x];
				std::bitset<_cache_width>& b_cached = _bit_masks[(2 * board_len) + b];
				if (y_cached[digit] | x_cached[digit] | b_cached[digit]) {
					return false;
				}

				std::bitset<_cache_width>& cell_digit_mask = _bit_masks[((3 * board_len) + board_size) +
					(digit * (board_len + 1)) + b
				];
				cell_digit_mask[bi] = 1;

				std::bitset<_cache_width>& cell_mask = _bit_masks[((3 * board_len) + board_size) +
					(0 * (board_len + 1)) + b
				];
				cell_mask[bi] = 1;

				y_cached[digit] = 1;
				x_cached[digit] = 1;
				b_cached[digit] = 1;

				known_locations++;
			}
			else {
				SelectionInfo& item = _selections[back_idx];
				item.col = x;
				item.row = y;
				item.blk = b;
				item.blk_idx = bi;
				item.value = 0;
				back_idx--;
			}
		}

		{
			size_t prev_count = 0;
			size_t mx_index = known_locations;

			uint32_t cell_idx = ((3 * board_len) + board_size);
			for (size_t idx = known_locations; idx < board_size; idx++) {
				SelectionInfo pick = _selections[idx];
				//performs faster on msvc...
				/*
				std::bitset<_cache_width> combined = _bit_masks[(0 * board_len) + pick.row];
				combined |= _bit_masks[(1 * board_len) + pick.col];
				combined |= _bit_masks[(2 * board_len) + pick.blk];

				_bit_masks[(3 * board_len) + ((pick.row * board_len) + pick.col)] = combined;
				*/
				std::bitset<_cache_width>& combined = _bit_masks[(3 * board_len) + ((pick.row * board_len) + pick.col)];
				combined = _bit_masks[(0 * board_len) + pick.row];
				combined |= _bit_masks[(1 * board_len) + pick.col];
				combined |= _bit_masks[(2 * board_len) + pick.blk];
				//_bit_masks[(0 * board_len) + pick.row];
				uint64_t* ptr = (uint64_t*)&combined;
				uint64_t count = _mm_popcnt_u64(*ptr);
				count += _mm_popcnt_u64(*(ptr + 1));
				count += _mm_popcnt_u64(*(ptr + 2));
				count += _mm_popcnt_u64(*(ptr + 3));

				_cell_count[(pick.row * board_len) + pick.col] = count;
				mx_index = (count > prev_count) ? idx : mx_index;
				prev_count = (count > prev_count) ? count : prev_count;
			}
			//we reserved +1 to do this unconditionally*
			std::swap(_selections[known_locations], _selections[mx_index]);
		}

		size_t index = known_locations;
		while (index >= known_locations && index < board_size) {
		start_stack:
			SelectionInfo& pick = _selections[index];
			std::bitset<_cache_width> s_mask = _bit_masks[(3 * board_len) + ((pick.row * board_len) + pick.col)];
			for (size_t digit = pick.value; ++digit <= board_len;) {
				if (s_mask[digit]) {
					continue;
				}
				(*num_guesses)++;

				_bit_masks[(0 * board_len) + pick.row][pick.value] = 0;
				_bit_masks[(1 * board_len) + pick.col][pick.value] = 0;
				_bit_masks[(2 * board_len) + pick.blk][pick.value] = 0;

				_bit_masks[(0 * board_len) + pick.row][digit] = 1;
				_bit_masks[(1 * board_len) + pick.col][digit] = 1;
				_bit_masks[(2 * board_len) + pick.blk][digit] = 1;

				pick.value = digit;

				goto next_stack;
			}
			//we don't jump here
		fail_stack:
			// failed to fill particular cell
			--index;
			if (index >= known_locations && index < board_size) {
				// unmark cell (backtrack)
				_bit_masks[(0 * board_len) + pick.row][pick.value] = 0;
				_bit_masks[(1 * board_len) + pick.col][pick.value] = 0;
				_bit_masks[(2 * board_len) + pick.blk][pick.value] = 0;
				pick.value = 0;
				//we already did our conditional test in theory should get optimized away
				//goto start_stack;
				continue;
			}
			else {
				// we walked back and undid the entire puzzle (impossible starting condition)
				return false;
			}
		next_stack:
			//prevent backtracking if we can show we have no choices left
			known_locations += (index == known_locations) && ((_cell_count[(pick.row * board_len) + pick.col] == (board_len - 1)) || pick.value == board_len);
			//cell_count[(pick.row * board_len) + pick.col]--;
			//resort by the least# of choices left to increase speed
			++index;
			if (index >= board_size) {
				break;
			}

			//do the dirty work of counting again
			size_t prev_count = 0;
			size_t mx_index = index;

			const size_t row_index = 0 * board_len;
			const size_t col_index = 1 * board_len;
			const size_t blk_index = 2 * board_len;
			const size_t msk_index = 3 * board_len;

			for (size_t idx = index; idx < board_size; idx++) {
				SelectionInfo pick = _selections[idx];
				//performs faster on msvc...
				size_t row_mask = row_index + pick.row;
				size_t col_mask = col_index + pick.col;
				size_t blk_mask = blk_index + pick.blk;

				uint32_t flat_index = ((pick.row * board_len) + pick.col);

				size_t msk_mask = msk_index + flat_index;
				/*
				std::bitset<_cache_width> combined = _bit_masks[row_mask];
				combined |= _bit_masks[col_mask];
				combined |= _bit_masks[blk_mask];
				_bit_masks[msk_mask] = combined;
				*/
				std::bitset<_cache_width>& combined = _bit_masks[msk_mask];
				combined = _bit_masks[row_mask];
				combined |= _bit_masks[col_mask];
				combined |= _bit_masks[blk_mask];
				//__m256i* ptr = (__m256i*)&combined;
				//*ptr = _mm256_load_si256((__m256i*)&_bit_masks[row_mask]);
				/*
				_mm256_storeu_si256((__m256i*)& combined, *(__m256i*)& _bit_masks[row_mask]);
				*(__m256i*)&combined = _mm256_or_si256(*(__m256i*)& combined, *(__m256i*)& _bit_masks[col_mask]);
				*(__m256i*)& combined = _mm256_or_si256(*(__m256i*) & combined, *(__m256i*) & _bit_masks[blk_mask]);
				* */
				/*
				std::bitset<_cache_width> combined;
				*(uint64_t*)&combined = *(uint64_t*)&_bit_masks[row_mask] | *(uint64_t*)&_bit_masks[col_mask] | *(uint64_t*)&_bit_masks[blk_mask];
				*(((uint64_t*)&combined) + 1) = *(((uint64_t*)&_bit_masks[row_mask]) + 1) | *(((uint64_t*)&_bit_masks[col_mask]) + 1) | *(((uint64_t*)&_bit_masks[blk_mask]) + 1);
				*(((uint64_t*)&combined) + 2) = *(((uint64_t*)&_bit_masks[row_mask]) + 2) | *(((uint64_t*)&_bit_masks[col_mask]) + 2) | *(((uint64_t*)&_bit_masks[blk_mask]) + 2);
				*(((uint64_t*)&combined) + 3) = *(((uint64_t*)&_bit_masks[row_mask]) + 3) | *(((uint64_t*)&_bit_masks[col_mask]) + 3) | *(((uint64_t*)&_bit_masks[blk_mask]) + 3);
				*/

				//uint8_t count = combined.count();
				uint64_t count = _mm_popcnt_u64(*(uint64_t*)&combined);
				count += _mm_popcnt_u64(*(((uint64_t*)&combined) + 1));
				count += _mm_popcnt_u64(*(((uint64_t*)&combined) + 2));
				count += _mm_popcnt_u64(*(((uint64_t*)&combined) + 3));

				_cell_count[flat_index] = count;

				mx_index = (count > prev_count) ? idx : mx_index;
				prev_count = (count > prev_count) ? count : prev_count;
			}
			std::swap(_selections[index], _selections[mx_index]);
			//std::swap(selections[index], *it);
		}

		//write out to puzzle
		for (size_t i = 0; i < board_size; i++) {
			SelectionInfo pick = _selections[i];
			solution[(pick.row * board_len) + pick.col] = ((uint8_t)pick.value + (uint8_t)'0');
			//board[pick.row][pick.col] = (uint8_t)pick.value + (uint8_t)'0';
		}
		return true;
	}

};

extern "C"
size_t sff_solver(const char* input, size_t limit, uint32_t configuration,
	char* solution, size_t * num_guesses) {
	static Solver solver;
	solver.solveSudoku(input, limit, configuration, solution, num_guesses);
	return *num_guesses;
}

extern "C"
size_t sff_solver_simple(const char* input, size_t limit, uint32_t configuration,
	char* solution, size_t * num_guesses) {
	static Solver solver;
	solver.solveSudokuSimple(input, limit, configuration, solution, num_guesses);
	return *num_guesses;
}

