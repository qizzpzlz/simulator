# Simulator Manual

## `class QueueAlgorithm`

interface 역할을 하는 abstract (pure virtual) class 로 custom algorithm 의 구현은 QueueAlgorithm class를 상속하는 class를 정의하는 방식으로 시작한다.

- `void QueueAlgorithm::run(std::vector<Job>& jobs) const` 

  새로운 알고리즘을 구현하기 위해서는`QueueAlgorithm` 의 virtual member function인  `run` 을 override 하여 함수를 작성해야 합니다. `run` 함수는 `std::vector<Job>& jobs` 를 argument로 가지고 있으며, 이는 현재 queue에 있는 모든 Job을 의미합니다.

  `Job::get_eligible_hosts()` 함수를 통해 현재 클러스터에서 특정 Job을 실행시킬 수 있는 모든 Host를 구할 수 있으며, `Host::execute_job()` 함수를 통해서 특정 Host에 특정 Job을 할당할 수 있습니다.

  함수 종료 시에 아무 Host에도 할당되지 않은 Job은 pending 상태가 되어 다음 dispatch 시까지 대기 상태에 놓입니다.

  만들어진 새로운 클래스를 `main.cpp` 에서 `ClusterSimulation` 의 constructor에 넣어주시면 됩니다.

## `class Job`

스케쥴러에서 Task에 해당하는 객체입니다.

- `std::vector<Host*> Job::get_eligible_hosts()`

  이 Job을 실행시킬 수 있는 host들의 목록을 반환합니다.



## `class Host`

- `void Host::execute_job(const Job& job)`

  `job`을 해당 Host 에 dispatch 하고 실행합니다.

- `ms get_expected_time_of_all_completion()`

  현재 Host에서 실행 중인 모든 Job이 종료되는 시점을 반환합니다.

- `std::chrono::milliseconds get_expected_run_time(const Job& job)`

  현재 Host에서 주어진 Job을 실행시켰을 때의 예상되는 run time의 길이를 반환합니다.