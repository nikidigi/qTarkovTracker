.PHONY: format

format:
	find . -type d \( -name build -o -name .git \) -prune -o \
		-type f \( -name '*.h' -o -name '*.cpp' \) \
		-exec clang-format -i {} +
