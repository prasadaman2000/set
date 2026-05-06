#include <sys/stat.h>
#include <sys/types.h>

#include <cctype>
#include <condition_variable>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_cat.h"
#include "game.h"
#include "player.h"

enum PlayerType {
  RANDOM,
  ORDERED,
  FORGETFUL,
  MATCH,
  REMEMBER,
  REMEMBER_FORGETFUL
};

bool AbslParseFlag(absl::string_view text, PlayerType* value,
                   std::string* error) {
  std::string flag;
  for (char c : text) {
    flag += std::toupper(c);
  }

  if (flag == "RANDOM") {
    *value = RANDOM;
  } else if (flag == "ORDERED") {
    *value = ORDERED;
  } else if (flag == "FORGETFUL") {
    *value = FORGETFUL;
  } else if (flag == "MATCH") {
    *value = MATCH;
  } else if (flag == "REMEMBER") {
    *value = REMEMBER;
  } else if (flag == "REMEMBER_FORGETFUL") {
    *value = REMEMBER_FORGETFUL;
  } else {
    *error = "bad bad bad";
    return false;
  }
  return true;
}

std::string AbslUnparseFlag(PlayerType value) {
  if (value == RANDOM) {
    return "RANDOM";
  }

  if (value == ORDERED) {
    return "ORDERED";
  }

  if (value == FORGETFUL) {
    return "FORGETFUL";
  }

  if (value == MATCH) {
    return "MATCH";
  }

  if (value == REMEMBER) {
    return "REMEMBER";
  }

  if (value == REMEMBER_FORGETFUL) {
    return "REMEMBER_FORGETFUL";
  }

  return "UNKNOWN";
}

ABSL_FLAG(int, num_games_to_run, 20,
          "Number of games to run as a benchmark.\n");

ABSL_FLAG(PlayerType, player_type, RANDOM, "Player type to use");

std::unique_ptr<set::Player> PlayerFlagToPlayer(set::Game* g) {
  switch (absl::GetFlag(FLAGS_player_type)) {
    case RANDOM:
      std::cout << "Creating RandomScanPlayer\n";
      return std::make_unique<set::RandomScanPlayer>(g);
    case ORDERED:
      std::cout << "Creating OrderedScanPlayer\n";
      return std::make_unique<set::OrderedScanPlayer>(g);
    case FORGETFUL:
      std::cout << "Creating ForgetfulScanPlayer\n";
      return std::make_unique<set::ForgetfulScanPlayer>(g);
    case MATCH:
      std::cout << "Creating MatchPlayer\n";
      return std::make_unique<set::MatchPlayer>(g);
    case REMEMBER:
      std::cout << "Creating RememberWhatIWantPlayer\n";
      return std::make_unique<set::RememberTheLastTurnPlayer>(g);
    case REMEMBER_FORGETFUL:
      std::cout << "Creating RememberWhatIWantPlayerForgetful\n";
      return std::make_unique<set::RememberTheLastTurnPlayerForgetful>(g);
  }
}

std::string PlayerTypeFromFlag() {
  return AbslUnparseFlag(absl::GetFlag(FLAGS_player_type));
}

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);

  std::mutex mu;
  std::condition_variable cv;
  int thread_count = 0;
  constexpr int kMaxThreadCount = 4;
  std::vector<set::Result> results;
  std::vector<std::thread> threads;

  for (int i = 0; i < absl::GetFlag(FLAGS_num_games_to_run); ++i) {
    {
      std::unique_lock<std::mutex> lock(mu);
      cv.wait(lock, [&thread_count] { return thread_count < kMaxThreadCount; });
      ++thread_count;
      threads.push_back(std::thread([&mu, &thread_count, &results, &cv] {
        set::Game g;
        auto p = PlayerFlagToPlayer(&g);
        set::Result result = g.Play(p.get());
        {
          std::unique_lock<std::mutex> lock(mu);
          results.push_back(result);
          std::cout << "finished game #" << results.size() << "\n";
          --thread_count;
          cv.notify_all();
        }
      }));
    }
  }

  for (std::thread& t : threads) {
    t.join();
  }

  absl::Time now = absl::Now();
  absl::TimeZone timezone = absl::LocalTimeZone();

  std::string sub_directory =
      absl::StrCat(absl::FormatTime("%Y_%m_%d_%H_%M_%S_", now, timezone),
                   PlayerTypeFromFlag());
  std::string dir_path =
      absl::StrCat("/Users/aman/CS/set/results/", sub_directory);
  mkdir(dir_path.c_str(), 0777);

  for (int i = 0; i < results.size(); ++i) {
    std::string path = absl::StrCat(dir_path, "/game_", i, ".csv");
    std::ofstream out_file(path);
    out_file << "turn,time_ms,move,board_id\n";
    for (int j = 0; j < results[i].move_times.size(); ++j) {
      out_file << j + 1 << ","
               << absl::ToDoubleMilliseconds(results[i].move_times[j]) << ","
               << results[i].moves[j] << "," << results[i].state_ids[j] << "\n";
    }
    std::cout << "Game " << i + 1 << " time " << results[i].total_game_time
              << "\n";
  }
}
