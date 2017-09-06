//
//  ConfigurationViewController.m
//  SpectREM
//
//  Created by Mike Daley on 06/09/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#import "ConfigurationViewController.h"

#pragma mark - Key Path Constants

NSString *const cSELECTED_MACHINE               = @"selectedMachine";
NSString *const cDISPLAY_BORDER_WIDTH           = @"displayBorderWidth";
NSString *const cDISPLAY_FILTER_VALUE           = @"displayFilterValue";

@interface ConfigurationViewController ()

@property (strong) NSUserDefaultsController *userDefaultController;

@end

@implementation ConfigurationViewController

- (instancetype)initWithCoder:(NSCoder *)coder
{
    self = [super initWithCoder:coder];
    if (self)
    {
        self.userDefaultController = [NSUserDefaultsController sharedUserDefaultsController];
        NSString *userDefaultsPath = [[NSBundle mainBundle] pathForResource:@"Defaults" ofType:@"plist"];
        NSDictionary *userDefaults = [NSDictionary dictionaryWithContentsOfFile:userDefaultsPath];
        [[NSUserDefaults standardUserDefaults] registerDefaults:userDefaults];
        [self.userDefaultController setInitialValues:userDefaults];
        [self setupBindings];
    }
    return self;
}

- (void)setupBindings
{
//    [self bind:cSELECTED_MACHINE toObject:self.userDefaultController withKeyPath:@"values.selectedMachine" options:nil];
//    [self bind:cDISPLAY_BORDER_WIDTH toObject:self.userDefaultController withKeyPath:@"values.displayBorderWidth" options:nil];
//    [self bind:cDISPLAY_FILTER_VALUE toObject:self.userDefaultController withKeyPath:@"values.displayFilterValue" options:nil];
}

@end
