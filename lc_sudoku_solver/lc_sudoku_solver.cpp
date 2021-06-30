#if 1
#include <cstdint>
#include <vector>
#include <array>
#include <algorithm>
#include <iostream>
#include <chrono>

#include <bitset>

#include <limits>
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

class Selection {
public:
	uint8_t value = 0;
	uint8_t row = 0;
	uint8_t col = 0;
	uint8_t blk = 0;
	//std::bitset<32> mask;
	//uint32_t idx = 0;

	Selection(uint8_t v, uint8_t r, uint8_t c, uint8_t b) : value(v), row(r), col(c), blk(b)
	//	idx(0)
	{};
	Selection() = default;
}; 

class Solution {
public:
	void solveSudoku(std::vector<std::vector<char>>& board) {
		//ignore weird boards
		if (!board.size())
			return;
		if (board[0].size() != board.size())
			return;
		/*
		for (size_t y = 0; y < board.size(); y++) {
			for (size_t x = 0; x < board[0].size(); x++) {
				std::cout << board[y][x] << ' ';
			}
			std::cout << '\n';
		}
		*/
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
					/*
					selection_cache[(0 * cache_size) + (y * (board_width + 1)) + digit] = 1;
					selection_cache[(1 * cache_size) + (x * (board_width + 1)) + digit] = 1;
					selection_cache[(2 * cache_size) + (b * (board_width + 1)) + digit] = 1;
					*/
				} else {
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
#if 0
		std::sort(selections.begin() + known_locations, selections.end(), [&selection_cache, cache_size, board_len](Selection& lhs, Selection& rhs) {
			size_t lhs_marks = 0;
			size_t rhs_marks = 0;
			uint32_t cache_width = board_len + 1;
			//we can ignore 0 b/c it's the throw away index
			uint8_t* y_lhs = &selection_cache[(0 * cache_size) + (lhs.row * (cache_width))];
			uint8_t* x_lhs = &selection_cache[(1 * cache_size) + (lhs.col * (cache_width))];
			uint8_t* b_lhs = &selection_cache[(2 * cache_size) + (lhs.blk * (cache_width))];
			uint8_t* y_rhs = &selection_cache[(0 * cache_size) + (rhs.row * (cache_width))];
			uint8_t* x_rhs = &selection_cache[(1 * cache_size) + (rhs.col * (cache_width))];
			uint8_t* b_rhs = &selection_cache[(2 * cache_size) + (rhs.blk * (cache_width))];

			for (uint32_t digit = 1; digit < cache_width; digit++) {
				lhs_marks += (y_lhs[digit] || x_lhs[digit] || b_lhs[digit]);
				rhs_marks += (y_rhs[digit] || x_rhs[digit] || b_rhs[digit]);
				/*
				{
					uint8_t* y_cached = &selection_cache[(0 * cache_size) + (lhs.row * (cache_width)) + digit];
					uint8_t* x_cached = &selection_cache[(1 * cache_size) + (lhs.col * (cache_width)) + digit];
					uint8_t* b_cached = &selection_cache[(2 * cache_size) + (lhs.blk * (cache_width)) + digit];

					lhs_marks += *y_cached || *x_cached || *b_cached;
				}
				{
					uint8_t* y_cached = &selection_cache[(0 * cache_size) + (rhs.row * (cache_width)) + digit];
					uint8_t* x_cached = &selection_cache[(1 * cache_size) + (rhs.col * (cache_width)) + digit];
					uint8_t* b_cached = &selection_cache[(2 * cache_size) + (rhs.blk * (cache_width)) + digit];

					rhs_marks += *y_cached || *x_cached || *b_cached;
				}
				*/
			}
			return rhs_marks < lhs_marks;
		});
#endif
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
			/*
			uint32_t cache_width = board_len + 1;
			//we can ignore 0 b/c it's the throw away index
			uint8_t* y_lhs = &selection_cache[(0 * cache_size) + (lhs.row * (cache_width))];
			uint8_t* x_lhs = &selection_cache[(1 * cache_size) + (lhs.col * (cache_width))];
			uint8_t* b_lhs = &selection_cache[(2 * cache_size) + (lhs.blk * (cache_width))];
			uint8_t* y_rhs = &selection_cache[(0 * cache_size) + (rhs.row * (cache_width))];
			uint8_t* x_rhs = &selection_cache[(1 * cache_size) + (rhs.col * (cache_width))];
			uint8_t* b_rhs = &selection_cache[(2 * cache_size) + (rhs.blk * (cache_width))];

			for (uint32_t digit = 1; digit < cache_width; digit++) {
				lhs_marks += (y_lhs[digit] || x_lhs[digit] || b_lhs[digit]);
				rhs_marks += (y_rhs[digit] || x_rhs[digit] || b_rhs[digit]);
			}
			*/
			/*
			for (uint32_t digit = 1; digit < (board_len + 1); digit++) {
				{
					uint8_t* y_cached = &selection_cache[(0 * cache_size) + (lhs.row * (cache_width)) + digit];
					uint8_t* x_cached = &selection_cache[(1 * cache_size) + (lhs.col * (cache_width)) + digit];
					uint8_t* b_cached = &selection_cache[(2 * cache_size) + (lhs.blk * (cache_width)) + digit];

					lhs_marks += *y_cached || *x_cached || *b_cached;
				}
				{
					uint8_t* y_cached = &selection_cache[(0 * cache_size) + (rhs.row * (cache_width)) + digit];
					uint8_t* x_cached = &selection_cache[(1 * cache_size) + (rhs.col * (cache_width)) + digit];
					uint8_t* b_cached = &selection_cache[(2 * cache_size) + (rhs.blk * (cache_width)) + digit];

					rhs_marks += *y_cached || *x_cached || *b_cached;
				}
			}
			*/
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
			} else {
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

				/*
				uint32_t cache_width = board_len + 1;
				//we can ignore 0 b/c it's the throw away index
				for (uint32_t digit = 1; digit < (board_len + 1); digit++) {
					{
						uint8_t* y_cached = &selection_cache[(0 * cache_size) + (lhs.row * (cache_width)) + digit];
						uint8_t* x_cached = &selection_cache[(1 * cache_size) + (lhs.col * (cache_width)) + digit];
						uint8_t* b_cached = &selection_cache[(2 * cache_size) + (lhs.blk * (cache_width)) + digit];

						lhs_marks += *y_cached || *x_cached || *b_cached;
					}
					{
						uint8_t* y_cached = &selection_cache[(0 * cache_size) + (rhs.row * (cache_width)) + digit];
						uint8_t* x_cached = &selection_cache[(1 * cache_size) + (rhs.col * (cache_width)) + digit];
						uint8_t* b_cached = &selection_cache[(2 * cache_size) + (rhs.blk * (cache_width)) + digit];

						rhs_marks += *y_cached || *x_cached || *b_cached;
					}
				}
				*/
				return rhs_marks < lhs_marks;
			});
			if (index >= selections.size()) {
				break;
			}
			std::swap(selections[index], *it);
		}
		/*
		std::sort(selections.begin(), selections.end(), [board_len](Selection& lhs, Selection& rhs) {
			return (lhs.row * board_len + lhs.col) < (rhs.row * board_len + rhs.col);
		});
		*/
		//write out to puzzle
		for (size_t i = 0; i < selections.size(); i++) {
			Selection pick = selections[i];
			board[pick.row][pick.col] = (uint8_t)pick.value+(uint8_t)'0';
		}
		/*
		for (size_t y = 0; y < board.size(); y++) {
			for (size_t x = 0; x < board[0].size(); x++) {
				std::cout << board[y][x] << ' ';
			}
			std::cout << '\n';
		}
		*/
	}

	template<size_t N>
	void solveSudoku(std::array<std::array<char,N>,N>& board) {
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
		std::array<uint8_t, board_size * cache_width> cell_mask;
		std::array<uint8_t, board_size * cache_width> invalid_mask;
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
							invalid_mask[(((y * board_len) + x)*cache_width) + digit] = 1;

						tmp_idx++;
						*y_cached = 1;
						*x_cached = 1;
						*b_cached = 1;
					} else {
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
#if 0
		for (uint32_t idx = 0; idx < board_size; idx++) {
			Selection pick = selections[idx];
			//uint32_t ty = selections[idx].row;//idx / board_len;
			//uint32_t tx = selections[idx].col;// / idx % board_len;
			//uint32_t tb = selections[idx].blk;//calculate_block(tx, ty, blk_len);

			uint8_t* y_pick = &selection_cache[(0 * cache_size) + (pick.row * (cache_width))];
			uint8_t* x_pick = &selection_cache[(1 * cache_size) + (pick.col * (cache_width))];
			uint8_t* b_pick = &selection_cache[(2 * cache_size) + (pick.blk * (cache_width))];
			uint8_t* m_pick = &invalid_mask[((pick.row * board_len + pick.col) * cache_width)];

			uint32_t count = 0;

			//m_pick[0] = (selections[idx].value > 0);
			for (uint32_t digit = 1; digit < cache_width; digit++) {
				m_pick[digit] = (pick.value > 0) ||
					(y_pick[digit] || x_pick[digit] || b_pick[digit]);
				count += m_pick[digit];
			}

			cell_count[(pick.row * board_len) + pick.col] = count;
		}
		/*
		for (size_t idx = known_locations; idx < selections.size(); idx++) {
			Selection pick = selections[idx];
			uint32_t m_count = cell_count[(pick.row * board_len) + pick.col];
			
			for (uint32_t xy = 0; xy < board_len; xy++) {
				uint32_t bx = xy % blk_len;
				uint32_t by = xy / blk_len;
				uint32_t tx = blk_x + bx;
				uint32_t ty = blk_y + by;
				//s_pick[(((ty * board_len) + tx) * cache_width) + digit] |= cell_mask[(ty * board_len) + tx];
			}

		}
		*/
#endif		
#if 0
		for (size_t idx = known_locations; idx < selections.size(); idx++) {
			uint8_t count = 0;
			Selection pick = selections[idx];
			uint8_t* y_pick = &selection_cache[(0 * cache_size) + (pick.row * (cache_width))];
			uint8_t* x_pick = &selection_cache[(1 * cache_size) + (pick.col * (cache_width))];
			uint8_t* b_pick = &selection_cache[(2 * cache_size) + (pick.blk * (cache_width))];
			
			uint8_t* s_pick = &invalid_mask[((pick.row * board_len) + pick.col) * cache_width];
			uint8_t* m_mask = &cell_mask[(pick.row * board_len) + pick.col];

			for (uint32_t digit = 1; digit < cache_width; digit++) {
				count += (y_pick[digit] || x_pick[digit] || b_pick[digit]);
				s_pick[digit] = (y_pick[digit] || x_pick[digit] || b_pick[digit]);
			}
			/*
			uint32_t blk_y = (pick.row / blk_len) * blk_len;
			uint32_t blk_x = (pick.col / blk_len) * blk_len;
			for (uint32_t digit = 1; digit < cache_width; digit++) {
				for (uint32_t xy = 0; xy < board_len; xy++) {
					uint32_t bx = xy % blk_len;
					uint32_t by = xy / blk_len;
					uint32_t tx = blk_x + bx;
					uint32_t ty = blk_y + by;
					s_pick[(((ty * board_len) + tx) * cache_width) + digit] |= cell_mask[(ty * board_len) + tx];
				}
				//uint32_t bx = (digit - 1) % blk_len;
				//uint32_t by = (digit - 1) / blk_len;
				
			}
			*/
			/*
			for (uint32_t by = 0; by < blk_len; by++) {
				for (uint32_t bx = 0; bx < blk_len; bx++) {

				}
			}
			*/
			cell_count[(pick.row * board_len) + pick.col] = count;
		}
#endif
#if 1
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

				//i_info[i_info[0]+1] = digit;
				//i_info[0] += !is_marked;
				/*
				if (!(y_pick[digit] || x_pick[digit] || b_pick[digit])) {
					uint32_t marked = 0;
					for (uint32_t xy = 0; xy < board_len; xy++) {
						uint32_t bx = xy % blk_len;
						uint32_t by = xy / blk_len;
						uint32_t tx = blk_x + bx;
						uint32_t ty = blk_y + by;
						invalid_mask[(((ty * board_len) + tx) * cache_width) + digit] |= cell_mask[(ty * board_len) + tx];
						//s_pick[(((ty * board_len) + tx) * cache_width) + digit] |= cell_mask[(ty * board_len) + tx];
						marked += (bool)invalid_mask[(((ty * board_len) + tx) * cache_width) + digit];
							//s_pick[(((ty * board_len) + tx) * cache_width) + digit];
					}
					if (marked >= (board_len - 1) && count < marked && s_pick[digit] == 0) {
						count = marked;
						for (uint32_t digit2 = 1; digit2 < digit; digit2++) {
							//invalid_mask[(((pick.row * board_len) + pick.col) * cache_width) + digit]
							s_pick[digit2] = 1;
						}
						for (uint32_t digit2 = digit + 1; digit2 < cache_width; digit2++) {
							s_pick[digit2] = 1;
						}
						break;
						//invalid_mask[(((pick.row * board_len) + pick.col) * cache_width) + digit] = 0;
					}
				}
				*/
			}
			/*
			uint32_t blk_y = (pick.row / blk_len) * blk_len;
			uint32_t blk_x = (pick.col / blk_len) * blk_len;
			for (uint32_t digit = 1; digit < cache_width; digit++) {
				for (uint32_t xy = 0; xy < board_len; xy++) {
					uint32_t bx = xy % blk_len;
					uint32_t by = xy / blk_len;
					uint32_t tx = blk_x + bx;
					uint32_t ty = blk_y + by;
					s_pick[(((ty * board_len) + tx) * cache_width) + digit] |= cell_mask[(ty * board_len) + tx];
				}
				//uint32_t bx = (digit - 1) % blk_len;
				//uint32_t by = (digit - 1) / blk_len;

			}
			*/
			/*
			for (uint32_t by = 0; by < blk_len; by++) {
				for (uint32_t bx = 0; bx < blk_len; bx++) {

				}
			}
			*/
			cell_count[(pick.row * board_len) + pick.col] = count;
		}
#endif
		
		auto it = std::min_element(selections.begin() + known_locations, selections.end(), [&cell_count, board_len](Selection& lhs, Selection& rhs) {
			return cell_count[(rhs.row * board_len) + rhs.col] < cell_count[(lhs.row * board_len) + lhs.col];
			});
		if (known_locations < selections.size()) {
			std::swap(selections[known_locations], *it);
		}
		
		/*
		for (size_t idx = known_locations; idx < selections.size(); idx++) {
			Selection lhs = selections[known_locations];
			Selection rhs = selections[idx];
			uint32_t lhs_count = cell_count[(lhs.row * board_len) + lhs.col];
			uint32_t rhs_count = cell_count[(rhs.row * board_len) + rhs.col];
			if (rhs_count > lhs_count || rhs_count <= 1) {
				std::swap(selections[known_locations], selections[idx]);
			}
			known_locations += (rhs_count <= 1);
		}
		*/
		/*
		std::sort(selections.begin() + known_locations, selections.end(), [&cell_count, board_len](Selection& lhs, Selection& rhs) {
			return cell_count[(rhs.row * board_len) + rhs.col] < cell_count[(lhs.row * board_len) + lhs.col];
		});
		*/
		size_t index = known_locations;
		while (index >= known_locations && index < selections.size()) {
			Selection& pick = selections[index];

			uint8_t* y_pick = &selection_cache[(0 * cache_size) + (pick.row * (cache_width))];
			uint8_t* x_pick = &selection_cache[(1 * cache_size) + (pick.col * (cache_width))];
			uint8_t* b_pick = &selection_cache[(2 * cache_size) + (pick.blk * (cache_width))];
			uint8_t* s_mask = &invalid_mask[(((pick.row * board_len) + pick.col) * cache_width)];
			//uint8_t* i_info = &init_vals[(((pick.row * board_len) + pick.col) * cache_width)];
			//uint8_t* v_info = &init_vals[(((pick.row * board_len) + pick.col) * cache_width) + 1];
			//
			//uint32_t max_count = i_info[0];
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
				/*
				uint8_t* y_prev = &selection_cache[(0 * cache_size) + ((uint32_t)pick.row * (cache_width)) + pick.value];
				uint8_t* x_prev = &selection_cache[(1 * cache_size) + ((uint32_t)pick.col * (cache_width)) + pick.value];
				uint8_t* b_prev = &selection_cache[(2 * cache_size) + ((uint32_t)pick.blk * (cache_width)) + pick.value];
				*/
				y_pick[pick.value] = 0;
				x_pick[pick.value] = 0;
				b_pick[pick.value] = 0;
				//pick.idx = 0;
				cell_mask[(pick.value * board_size) + (pick.row * board_len) + pick.col] = 0;
				cell_mask[(pick.row * board_len) + pick.col] = 0;
				/*
				*y_prev = 0;
				*x_prev = 0;
				*b_prev = 0;
				*/
				pick.value = 0;
				continue;
			} else {
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
			//uint32_t max_count = 0;
			//bool bad_pick = false;
			
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
				/*
				for (size_t i = 0; i < board_size; i++) {
					uint32_t y = i / board_len;
					uint32_t x = i % board_len;
					uint32_t b = calculate_block(x, y, blk_len);

					uint8_t* y_pick = &selection_cache[(0 * cache_size) + (y * (cache_width))];
					uint8_t* x_pick = &selection_cache[(1 * cache_size) + (x * (cache_width))];
					uint8_t* b_pick = &selection_cache[(2 * cache_size) + (b * (cache_width))];
					//((y * board_len) + x)
					uint8_t* s_pick = &invalid_mask[i * cache_width];

					bool is_marked = (y_pick[digit] || x_pick[digit] || b_pick[digit]);
					cell_count[i] += is_marked;
					s_pick[digit] = is_marked;
				}
				*/
			}
			
			/*
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
			*/
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
						//selections.emplace_back((uint8_t)digit, (uint8_t)y, (uint8_t)x, (uint8_t)b);
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

					} else {
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
#if 1
		for (size_t idx = known_locations; idx < selections.size(); idx++) {
			Selection pick = selections[idx];
			std::bitset<cache_width> y_cached = bitwise_cache[(0 * board_len) + pick.row];
			std::bitset<cache_width> x_cached = bitwise_cache[(1 * board_len) + pick.col];
			std::bitset<cache_width> b_cached = bitwise_cache[(2 * board_len) + pick.blk];

			std::bitset<cache_width> combined = y_cached | x_cached | b_cached;
			bitwise_mask[((pick.row * board_len) + pick.col)] = combined;
			uint8_t count = combined.count();
			cell_count[(pick.row * board_len) + pick.col] = count;
		}
#endif

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
				// unmark cell
				bitwise_cache[(0 * board_len) + pick.row][pick.value] = 0;
				bitwise_cache[(1 * board_len) + pick.col][pick.value] = 0;
				bitwise_cache[(2 * board_len) + pick.blk][pick.value] = 0;
				pick.value = 0;
				continue;
			} else {
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
				} else {
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
};

#endif

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

	for (size_t y = 0; y < puzzle_arr.size(); y++) {
		for (size_t x = 0; x < puzzle_arr[0].size(); x++) {
			std::cout << puzzle_arr[y][x] << ' ';
		}
		std::cout << '\n';
	}

	Solution solver;
	auto ts_0 = std::chrono::steady_clock::now();
	//solver.solveSudoku(puzzle);
	solver.solveSudoku4(puzzle_arr);
	//solver.solveSudoku3(puzzle);
	auto ts_1 = std::chrono::steady_clock::now();

	std::cout << '\n';
	for (size_t y = 0; y < puzzle_arr.size(); y++) {
		for (size_t x = 0; x < puzzle_arr[0].size(); x++) {
			std::cout << puzzle_arr[y][x] << ' ';
		}
		std::cout << '\n';
	}

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
	}

	size_t ns = (ts_1 - ts_0).count();
	std::cout << "time (ns)\n";
	std::cout << ns << '\n';
	std::cout << "time (ms)\n";
	std::cout << (double)ns / (double)1e6 << '\n';
	

	return 0;
}
