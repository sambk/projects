# CADE

CADE (Cards Dealing) is a small library to implement a scenario "Poker table" as below. Let's assume that you have some nodes running some business tasks and a poker card set. Each node occupies and serves some resources named by the poker card set. And the constrain is as below:
```
No node has the same card.
If a node is down, the resources served by the node is moved to then served by other nodes so that the service is not interupted. And the change is the least.
If a new node is up and join, resource is redistributed to this node so that the number os resource are the most balanced. And the chage is the least.
A node is discovered automatically without hard configuration
```
To do that, CADE implement 3 role
```
Dealer: To manage distributing cards for players
Player: Occupies some cards
Watcher: Just get the information, not participate as player role
```


## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes. See deployment for notes on how to deploy the project on a live system.

### Prerequisites

What things you need to install the software and how to install them

```
linux
gcc
```

### Installing

A step by step series of examples that tell you how to get a development env running

Enter cade foler
```
cd cade
```

Compile

```
make
```

Complie examples
```
cd examples/dealer
make
cd examples/player
make
cd examplers/watcher
make
```

## Running the tests

To run one dealer and M players, N watchers:
```
./build/test_cade_dealer <dealer local ip> M N
./build/test_cade_player <player local ip>
./build/test_cade_watcher <watcher local ip>
```

## Deployment

CADE can be used as a C library with the way similar as examples


## Contributing

Please read [CONTRIBUTING.md](https://gist.github.com/sampv) for details on our code of conduct, and the process for submitting pull requests to us.

## Versioning

We use [SemVer](http://semver.org/) for versioning. For the versions available, see the [tags on this repository](https://github.com/your/project/tags). 

## Authors

* **Sam Pham Viet** - *Initial work* - [PurpleBooth](https://github.com/sampv)

See also the list of [contributors](https://github.com/your/project/contributors) who participated in this project.

## License

This project is an open source with free using
