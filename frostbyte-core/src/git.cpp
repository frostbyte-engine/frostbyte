#include "git.hpp"

#include <git2.h>

namespace frostbyte {

void gitInit() {
    git_libgit2_init();
}
void gitShutdown() {
    git_libgit2_shutdown();
}

std::optional<std::string> gitShallowClone(const char* url, const char* output_path) {
    git_clone_options opts;
    int rc = git_clone_options_init(&opts, GIT_CLONE_OPTIONS_VERSION);
    if (rc != 0) {
        const git_error* e = git_error_last();
        return e && e->message ? e->message : "unknown error when initializing options";
    }

    opts.checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE;

    opts.fetch_opts.depth = 1;

    git_repository* repo = nullptr;
    rc = git_clone(&repo, url, output_path, &opts);

    if (rc != 0) {
        const git_error* e = git_error_last();
        return e && e->message ? e->message : "unknown error when cloning";
    }

    git_repository_free(repo);

    return std::nullopt;
}

}
