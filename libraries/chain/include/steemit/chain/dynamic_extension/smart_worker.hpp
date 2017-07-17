#ifndef GOLOS_SMART_WORKER_HPP
#define GOLOS_SMART_WORKER_HPP

class smart_worker {
public:
    smart_worker(worker_t *ptr) : ptr(ptr) {
    }

private:
    worker_t *ptr;
};


#endif //GOLOS_SMART_WORKER_HPP
