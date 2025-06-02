# 🐍 ARM-Optimized Snake Game

[![Language: C](https://img.shields.io/badge/Language-C-blue.svg)](<https://en.wikipedia.org/wiki/C_(programming_language)>)
[![Platform: ARM](https://img.shields.io/badge/Platform-ARM%2032bit-red.svg)](https://www.arm.com/)
[![Terminal: ncurses](https://img.shields.io/badge/Terminal-ncurses-green.svg)](https://invisible-island.net/ncurses/)

ARM 어셈블리 최적화가 적용될 터미널 기반 스네이크 게임 🚀

## NOTION

https://www.notion.so/SYS_PROGRAMMING-Project-201abb72117c80e681a7da3bbe2f8c40?source=copy_link

## ✨ 특징

- 🔥 **ARM 32비트 어셈블리 최적화**: 핵심 로직을 어셈블리로 구현하여 성능 개선 (예정)
- 🎯 **스코어 시스템**: 먹이를 먹을 때마다 점수와 길이 증가
- 🔄 **게임 재시작**: 게임 오버 후 즉시 재시작 가능

## 🚀 빠른 시작

### 사전 요구사항

vi ~/.profile >> TERM=xterm-color 추가해주세요.
cat > dot.c 가 ucrt 환경에서는 잘 안될 수 있습니다. 
putty 를 다운로드 하시고 debian linux를 켜신 후에 ip: localhost, port:2222 로 접속해서 진행해주세요.

```bash
# Debian Linux
apt-get update
apt-get install libncurses-dev

```

### 컴파일 및 실행

```bash
# 저장소 클론
git clone https://github.com/cjh030808/sys_programming.git
cd arm-snake-game

# ARM 최적화 컴파일 (권장)
gcc -marm -O3 snake.c -o snake -lncurses


# 게임 실행
./snake
```

## 🎮 게임 조작법

| 키        | 동작                       |
| --------- | -------------------------- |
| `↑` / `W` | 위로 이동                  |
| `↓` / `S` | 아래로 이동                |
| `←` / `A` | 왼쪽으로 이동              |
| `→` / `D` | 오른쪽으로 이동            |
| `Q`       | 게임 종료                  |
| `R`       | 게임 재시작 (게임 오버 후) |

## ⚡ 성능 최적화(예정)

### ARM 어셈블리 최적화 함수들

1. **`clear_bitmap_asm()`** - 메모리 클리어 최적화

2. **`wrap_coordinate_asm()`** - 경계 처리 최적화

3. **`check_collision_asm()`** - 충돌 검사 최적화

4. **`move_snake_body_asm()`** - 배열 복사 최적화

## 🏗️ 프로젝트 구조

```
SYS_PROGRAMMING/
├── dot.c                # snake game을 만들기 위해 참고한 기본 소스코드
├── snake.c              # ARM 최적화를 진행한 메인 게임 소스코드(예정)
├── snake_basic.c        # ARM 최적화 없는 기본 버전
├── README.md            # README
└── docs/                # 문서(예정)
│   └──  ASSEMBLY.md     # 어셈블리 최적화 상세 설명(예정)
└── assets/              # 스크린샷 및 데모(예정)
    ├── demo.mp4
    └── screenshot.png
```

### 개발 가이드라인

- 모든 새로운 기능에는 테스트 추가
- 코드 스타일은 기존 코드를 따라주세요

## 📬 연락처

- 이슈: [GitHub Issues](https://github.com/cjh030808/sys_programming/issues)
- 학교 내 이메일: cjh030808@kau.kr
- 외부 contact: goodjjh1234@gmail.com

---

Made with C and ARM Assembly
